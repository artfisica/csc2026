#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

namespace sd_e2 {

// Array of Structures (AoS)
struct ParticleAoS {
    double px{};
    double py{};
    double pz{};
    double mass{};
};

struct ParticlesAoS {
    std::vector<ParticleAoS> particles;

    void resize(size_t n) { particles.resize(n); }

    double sumEnergy() const {
        double sum = 0.0;
        for (const auto& p : particles) {
            // E = sqrt(p^2 + m^2)
            const double p2 = p.px * p.px + p.py * p.py + p.pz * p.pz;
            sum += std::sqrt(p2 + p.mass * p.mass);
        }
        return sum;
    }
};

// Structure of Arrays (SoA) - intentionally suboptimal baseline
struct ParticlesSoA {
    std::vector<double> px;
    std::vector<double> py;
    std::vector<double> pz;
    std::vector<double> mass;

    void resize(size_t n) {
        px.resize(n);
        py.resize(n);
        pz.resize(n);
        mass.resize(n);
    }

    double sumEnergy() const {
        double sum = 0.0;

        // Prevent GCC/Clang from turning pow(x, 2) into x*x at -O3.
        // We keep the math identical (still exponent 2.0), but make it a runtime value.
        static volatile double two = 2.0;
        const double exp = two;

        for (size_t i = 0; i < px.size(); ++i) {
            const double p2 =
                std::pow(px[i], exp) +
                std::pow(py[i], exp) +
                std::pow(pz[i], exp);

            sum += std::sqrt(p2 + std::pow(mass[i], exp));
        }
        return sum;
    }
};

} // namespace sd_e2
