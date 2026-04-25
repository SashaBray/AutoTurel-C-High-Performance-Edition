#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include <cmath>
#include <iostream>

// Легкая структура вектора для максимальной скорости
// Light vector structure for maximum performance
struct Vector3 {
    double x, y, z;

    Vector3 operator+(const Vector3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vector3 operator-(const Vector3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vector3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vector3 operator-() const { return {-x, -y, -z}; }
    
    double length() const { return std::sqrt(x*x + y*y + z*z); }
    static Vector3 normalize(const Vector3& v) {
        double len = v.length();
        return (len > 0) ? v * (1.0 / len) : Vector3{0,0,0};
    }
    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    }
};

class Projectile {
public:
    Vector3 position;
    Vector3 velocity;
    double time = 0;

    Projectile(Vector3 startPos, Vector3 startVel, double bc, double derK, double lat, double planetR);

    void update(double dt, double density, double soundSpeed, double cd, const Vector3& wind);

private:
    double _bc, _derK, _pRadius;
    Vector3 _earthRotationVector; // Сила Кориолиса
};

#endif
