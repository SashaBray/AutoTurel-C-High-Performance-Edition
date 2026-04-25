#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <map>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- СТРУКТУРЫ ДАННЫХ ---
struct Vector3 {
    double x, y, z;
    Vector3 operator+(const Vector3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vector3 operator-(const Vector3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vector3 operator*(double s) const { return {x * s, y * s, z * s}; }
    double length() const { return std::sqrt(x*x + y*y + z*z); }
    static Vector3 normalize(const Vector3& v) {
        double len = v.length();
        return (len > 1e-9) ? v * (1.0 / len) : Vector3{0,0,0};
    }
    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    }
    static double dot(const Vector3& a, const Vector3& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
};

struct BallisticResult {
    double pitch, yaw, miss, angMiss;
    long long timeMs;
};

// --- ПРОВАЙДЕР СОПРОТИВЛЕНИЯ ---
class DragProvider {
public:
    static std::vector<std::pair<double, double>> customTable;

    static double getCd(const std::string& model, double mach) {
        if (model == "custom" && !customTable.empty()) {
            return interpolate(customTable, mach);
        }

        static const std::map<std::string, std::vector<std::pair<double, double>>> tables = {
            {"G1", {{0.0,0.2629},{0.5,0.2629},{0.9,0.274},{1.0,0.38},{1.2,0.468},{2.0,0.398},{4.0,0.279}}},
            {"G7", {{0.0,0.125},{0.9,0.14},{1.0,0.24},{1.2,0.35},{2.0,0.33},{4.0,0.26}}}
        };

        const auto& table = (tables.count(model)) ? tables.at(model) : tables.at("G1");
        return interpolate(table, mach);
    }

    // Быстрый парсер строки формата "M:Cd,M:Cd..."
    static void parseTable(const std::string& data) {
        customTable.clear();
        std::stringstream ss(data);
        std::string segment;
        while (std::getline(ss, segment, ',')) {
            size_t colon = segment.find(':');
            if (colon != std::string::npos) {
                double m = std::stod(segment.substr(0, colon));
                double c = std::stod(segment.substr(colon + 1));
                customTable.push_back({m, c});
            }
        }
        std::sort(customTable.begin(), customTable.end());
    }

private:
    static double interpolate(const std::vector<std::pair<double, double>>& table, double mach) {
        if (mach <= table.front().first) return table.front().second;
        if (mach >= table.back().first) return table.back().second;
        auto it = std::lower_bound(table.begin(), table.end(), mach, 
            [](const std::pair<double, double>& p, double v) { return p.first < v; });
        auto prev = std::prev(it);
        return prev->second + (mach - prev->first) * (it->second - prev->second) / (it->first - prev->first);
    }
};

std::vector<std::pair<double, double>> DragProvider::customTable;

// --- ЦЕЛЬ И СНАРЯД (Без изменений) ---
class Target {
public:
    Vector3 pos, vel, acc;
    Target(Vector3 p, Vector3 v, Vector3 a) : pos(p), vel(v), acc(a) {}
    void update(double dt) {
        if (vel.length() > 1e-6) {
            Vector3 oldDir = Vector3::normalize(vel);
            vel = vel + acc * dt;
            Vector3 newDir = Vector3::normalize(vel);
            double cosA = std::clamp(Vector3::dot(oldDir, newDir), -1.0, 1.0);
            double angle = std::acos(cosA);
            if (angle > 1e-6) {
                Vector3 axis = Vector3::normalize(Vector3::cross(oldDir, newDir));
                double c = std::cos(angle), s = std::sin(angle);
                acc = acc * c + Vector3::cross(axis, acc) * s + axis * Vector3::dot(axis, acc) * (1-c);
            }
        } else { vel = vel + acc * dt; }
        pos = pos + vel * dt;
    }
};

class Projectile {
public:
    Vector3 pos, vel;
    double time = 0;
    Projectile(Vector3 startPos, Vector3 startVel, double bc, double derK, double lat, double nAngle, double pRad) 
        : pos(startPos), vel(startVel), _bc(bc), _derK(derK), _pRadius(pRad) {
        double omega = 7.292115e-5, latR = lat * M_PI / 180.0, nR = nAngle * M_PI / 180.0;
        Vector3 geoOmega = { std::cos(latR) * omega, 0, std::sin(latR) * omega };
        double s = std::sin(-nR), c = std::cos(-nR);
        _earthRot = { geoOmega.x * c + geoOmega.y * s, -geoOmega.x * s + geoOmega.y * c, geoOmega.z };
    }
    void update(double dt, const std::string& model, double pRad, const Vector3& wind, double tB, double pB, double hum) {
        const double S_std = 0.0005064, M_std = 0.4536;
        Vector3 pCenter = {0, 0, -pRad};
        double alt = (pos - pCenter).length() - pRad;
        double Tc = tB - 0.0065 * alt, Tk = Tc + 273.15;
        if (Tk < 1.0) Tk = 1.0;
        double P = pB * std::pow(Tk / (tB + 273.15), 5.255);
        double Es = 6.1078 * std::exp((17.27 * Tc) / (Tc + 237.3)) * 100.0;
        double Pv = (hum / 100.0) * Es, Pd = P - Pv;
        double dens = (Pd / (287.05 * Tk)) + (Pv / (461.49 * Tk));
        double sound = std::sqrt(1.4 * 287.05 * Tk);
        Vector3 accGravity = Vector3::normalize(pCenter - pos) * 9.81;
        Vector3 relV = vel - wind; double vM = relV.length();
        Vector3 accDrag = (vM > 0.1) ? Vector3::normalize(relV) * -((0.5 * dens * vM * vM * DragProvider::getCd(model, vM/sound) * S_std) / (M_std * _bc)) : Vector3{0,0,0};
        Vector3 accCor = Vector3::cross(_earthRot, vel) * -2.0;
        Vector3 drift = Vector3::cross(vel, accGravity);
        Vector3 accDer = (drift.length() > 1e-6) ? Vector3::normalize(drift) * (vel.length() * _derK) : Vector3{0,0,0};
        vel = vel + (accGravity + accDrag + accCor + accDer) * dt;
        pos = pos + vel * dt; time += dt;
    }
private: double _bc, _derK, _pRadius; Vector3 _earthRot;
};

// --- DLL API ---
extern "C" {
    __declspec(dllexport) BallisticResult SolveBallistics(
        double tx, double ty, double tz, double vx, double vy, double vz, double ax, double ay, double az,
        double ox, double oy, double oz, double wx, double wy, double wz,
        double v0, int iters, bool high, double bc, double derivK, double lat, double nAngle,
        double temp, double press, double hum, double pRad, const char* dragModel, double convF, double mTime
    ) {
        auto startC = std::chrono::high_resolution_clock::now();
        Vector3 tp={tx,ty,tz}, tv={vx,vy,vz}, ta={ax,ay,az}, gunOff={ox,oy,oz}, wind={wx,wy,wz};
        std::string dM = dragModel;
        Vector3 aimP = tp; double p = high ? (45.0 * M_PI / 180.0) : std::asin(std::clamp((tz - oz) / std::max((tp - gunOff).length(), 0.1), -1.0, 1.0));
        double y = std::atan2(ty - oy, tx - ox);
        double fMiss = 0; Vector3 lastProjPos = gunOff;

        for (int i = 0; i < iters; ++i) {
            Vector3 sVel = {v0*std::cos(p)*std::cos(y), v0*std::cos(p)*std::sin(y), v0*std::sin(p)};
            Projectile proj(gunOff, sVel, bc, derivK, lat, nAngle, pRad); Target target(tp, tv, ta);
            double dIter = 1e18;
            while (true) {
                double altP = (proj.pos - Vector3{0,0,-pRad}).length() - pRad;
                double altT = (target.pos - Vector3{0,0,-pRad}).length() - pRad;
                proj.update(0.005, dM, pRad, wind, temp, press, hum); target.update(0.005);
                double d = (proj.pos - target.pos).length();
                if (high) { if (proj.vel.z < 0 && altP <= altT) break; }
                else { if (proj.time > 0.05 && d > dIter) break; }
                if (proj.time > mTime || altP < -500 || std::isnan(d)) break;
                dIter = d;
            }
            lastProjPos = proj.pos; fMiss = (target.pos - proj.pos).length();
            if (high) {
                double d_act = std::sqrt(proj.pos.x*proj.pos.x + proj.pos.y*proj.pos.y);
                double d_tar = std::sqrt(target.pos.x*target.pos.x + target.pos.y*target.pos.y);
                double aD = p * 180.0 / M_PI;
                double D = d_act / (1.0425 - 1.0426 * std::pow(aD / 90.0, 3));
                double nP = std::pow(std::max(0.0, (1.0426*D - d_tar)*std::pow(90,3)/(1.0426*D)), 1.0/3.0)*M_PI/180.0;
                p += (nP - p)*convF; y += (std::atan2(target.pos.y, target.pos.x) - std::atan2(proj.pos.y, proj.pos.x))*convF;
            } else {
                aimP = aimP + (target.pos - proj.pos) * convF;
                Vector3 relDir = aimP - gunOff;
                p = std::asin(std::clamp(relDir.z / std::max(relDir.length(), 0.1), -1.0, 1.0));
                y = std::atan2(relDir.y, relDir.x);
            }
        }
        Vector3 vT = Vector3::normalize(tp - gunOff), vP = Vector3::normalize(lastProjPos - gunOff);
        double fAngM = std::acos(std::clamp(Vector3::dot(vT, vP), -1.0, 1.0)) * 180.0 / M_PI;
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startC).count();
        return { p * 180.0 / M_PI, y * 180.0 / M_PI, fMiss, fAngM, dur };
    }
}

// --- CONSOLE MAIN ---
int main(int argc, char* argv[]) {
    Vector3 tp={1000,0,0}, tv={0,0,0}, ta={0,0,0}, gunOff={0,0,0}, wind={0,0,0};
    double v0=820, bc=0.5, derK=0.0001, lat=55, nA=0, pR=6371000, cF=0.8, mT=150, tB=15, pB=101325, hum=50;
    int iters=15; bool high=false; std::string dM="G1";

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--target-pos") sscanf(argv[++i], "%lf,%lf,%lf", &tp.x, &tp.y, &tp.z);
        else if (a == "--target-vel") sscanf(argv[++i], "%lf,%lf,%lf", &tv.x, &tv.y, &tv.z);
        else if (a == "--target-acc") sscanf(argv[++i], "%lf,%lf,%lf", &ta.x, &ta.y, &ta.z);
        else if (a == "--gun-offset") sscanf(argv[++i], "%lf,%lf,%lf", &gunOff.x, &gunOff.y, &gunOff.z);
        else if (a == "--wind") sscanf(argv[++i], "%lf,%lf,%lf", &wind.x, &wind.y, &wind.z);
        else if (a == "--v0") v0 = std::atof(argv[++i]);
        else if (a == "--bc") bc = std::atof(argv[++i]);
        else if (a == "--deriv-k") derK = std::atof(argv[++i]);
        else if (a == "--lat") lat = std::atof(argv[++i]);
        else if (a == "--north-angle") nA = std::atof(argv[++i]);
        else if (a == "--temp") tB = std::atof(argv[++i]);
        else if (a == "--press") pB = std::atof(argv[++i]);
        else if (a == "--hum") hum = std::atof(argv[++i]);
        else if (a == "--iters") iters = std::atoi(argv[++i]);
        else if (a == "--max-time") mT = std::atof(argv[++i]);
        else if (a == "--high-angle") high = true;
        else if (a == "--drag-model") dM = argv[++i];
        else if (a == "--drag-table") {
            DragProvider::parseTable(argv[++i]);
            dM = "custom";
        }
    }

    BallisticResult res = SolveBallistics(tp.x, tp.y, tp.z, tv.x, tv.y, tv.z, ta.x, ta.y, ta.z,
        gunOff.x, gunOff.y, gunOff.z, wind.x, wind.y, wind.z, v0, iters, high, bc, derK, lat, nA, tB, pB, hum, pR, dM.c_str(), cF, mT);

    std::cout << std::fixed << std::setprecision(4) << res.pitch << ";" << res.yaw << ";" << res.miss << ";" << res.angMiss << ";" << res.timeMs << std::endl;
    return 0;
}
