#ifndef ATMOSPHERE_HPP
#define ATMOSPHERE_HPP

#include <cmath>

struct AtmoState {
    double density;
    double soundSpeed;
};

class Atmosphere {
public:
    Atmosphere(double tempBase, double pressBase) 
        : _tBase(tempBase + 273.15), _pBase(pressBase) {}

    AtmoState getState(double altitude) const {
        const double L = 0.0065;  // Temperature lapse rate
        const double R = 287.05;  // Gas constant
        const double g = 9.80665;

        double T = _tBase - L * altitude;
        if (T < 1.0) T = 1.0;

        double P = _pBase * std::pow(T / _tBase, g / (R * L));
        double rho = P / (R * T);
        double c = std::sqrt(1.4 * R * T);

        return { rho, c };
    }

private:
    double _tBase, _pBase;
};

#endif
