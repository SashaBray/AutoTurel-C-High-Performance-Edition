#include "Projectile.hpp"

Projectile::Projectile(Vector3 startPos, Vector3 startVel, double bc, double derK, double lat, double planetR) 
    : position(startPos), velocity(startVel), _bc(bc), _derK(derK), _pRadius(planetR) {
    
    double omega = 7.292115e-5;
    double latRad = lat * M_PI / 180.0;
    
    // Упрощенный вектор вращения для теста
    _earthRotationVector = { std::cos(latRad) * omega, 0, std::sin(latRad) * omega };
}

void Projectile::update(double dt, double dens, double sound, double cd, const Vector3& wind) {
    const double S_std = 0.0005064;
    const double M_std = 0.4536;

    // Гравитация к центру планеты
    Vector3 pCenter = {0, 0, -_pRadius};
    Vector3 toCenter = pCenter - position;
    Vector3 accGravity = Vector3::normalize(toCenter) * 9.81;

    // Сопротивление
    Vector3 relV = velocity - wind;
    double vLen = relV.length();
    Vector3 accDrag = {0,0,0};
    if (vLen > 0.1) {
        double dMag = (0.5 * dens * vLen * vLen * cd * S_std) / (M_std * _bc);
        accDrag = Vector3::normalize(relV) * (-dMag);
    }

    // Кориолис и Деривация
    Vector3 accCor = Vector3::cross(_earthRotationVector, velocity) * (-2.0);
    Vector3 driftDir = Vector3::cross(velocity, accGravity);
    Vector3 accDer = (driftDir.length() > 1e-6) ? Vector3::normalize(driftDir) * (vLen * _derK) : Vector3{0,0,0};

    // Интегрирование
    velocity = velocity + (accGravity + accDrag + accCor + accDer) * dt;
    position = position + velocity * dt;
    time += dt;
}
