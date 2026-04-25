#include "Projectile.hpp"
#include "Atmosphere.hpp"
#include "DragProvider.hpp"
#include <chrono>
#include <vector>

struct SolveResult {
    double pitch, yaw, miss, totalTimeUs;
};

class BallisticSolver {
public:
    SolveResult solve(Vector3 tPos, Vector3 tVel, double v0, int iters, bool highAngle) {
        auto startClock = std::chrono::high_resolution_clock::now();
        
        double currentPitch = highAngle ? 45.0 * M_PI / 180.0 : 0.0;
        double currentYaw = std::atan2(tPos.y, tPos.x);
        double finalMiss = 0;

        Atmosphere atmo(15.0, 101325.0);
        DragProvider drag;
        drag.useEmbedded();

        for (int i = 0; i < iters; ++i) {
            Vector3 startVel = {
                v0 * std::cos(currentPitch) * std::cos(currentYaw),
                v0 * std::cos(currentPitch) * std::sin(currentYaw),
                v0 * std::sin(currentPitch)
            };

            // Симуляция выстрела
            Projectile p({0,0,0}, startVel, 0.5, 0.0001, 55.0, 6371000.0);
            double dt = 0.005;
            double lastDist = 1e18;

            while (true) {
                double alt = std::sqrt(p.position.x*p.position.x + p.position.y*p.position.y + (p.position.z + 6371000.0)*(p.position.z + 6371000.0)) - 6371000.0;
                
                AtmoState state = atmo.getState(alt);
                double cd = drag.getCd("G1", p.velocity.length() / state.soundSpeed);
                
                p.update(dt, state.density, state.soundSpeed, cd, {0,0,0});

                double d = (p.position - tPos).length(); // Упрощенно для статики
                
                if (highAngle) {
                    if (p.velocity.z < 0 && alt <= 0) break;
                } else {
                    if (d > lastDist) break;
                }
                
                if (p.time > 100.0 || alt < -100) break;
                lastDist = d;
            }

            Vector3 missVec = tPos - p.position;
            finalMiss = missVec.length();

            // Коррекция (настильная для примера)
            if (!highAngle) {
                currentPitch += missVec.z * 0.0001; // Упрощенный шаг для теста скорости
                currentYaw += missVec.y * 0.0001;
            }
        }

        auto endClock = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::microseconds>(endClock - startClock).count();

        return { currentPitch * 180.0 / M_PI, currentYaw * 180.0 / M_PI, finalMiss, duration };
    }
};
