#ifndef DRAGPROVIDER_HPP
#define DRAGPROVIDER_HPP

#include <vector>
#include <string>
#include <map>

struct DragPoint {
    double mach;
    double cd;
};

class DragProvider {
public:
    // Встроенная таблица G1 для теста скорости
    void useEmbedded() {
        _tables["G1"] = {
            {0.0, 0.262}, {0.5, 0.256}, {1.0, 0.450}, 
            {1.5, 0.420}, {2.0, 0.360}, {4.0, 0.270}
        };
    }

    double getCd(const std::string& model, double mach) const {
        auto it = _tables.find(model);
        if (it == _tables.end()) return 0.2;

        const auto& table = it->second;
        if (mach <= table.front().mach) return table.front().cd;
        if (mach >= table.back().mach) return table.back().cd;

        // Линейная интерполяция
        for (size_t i = 0; i < table.size() - 1; ++i) {
            if (mach >= table[i].mach && mach <= table[i+1].mach) {
                double m1 = table[i].mach, m2 = table[i+1].mach;
                double c1 = table[i].cd,   c2 = table[i+1].cd;
                return c1 + (mach - m1) * (c2 - c1) / (m2 - m1);
            }
        }
        return 0.2;
    }

private:
    std::map<std::string, std::vector<DragPoint>> _tables;
};

#endif
