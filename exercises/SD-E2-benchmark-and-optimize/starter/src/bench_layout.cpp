#include <benchmark/benchmark.h>

#include "ParticleLayouts.hpp"
#include <vector>

static void BM_AoS_SumEnergy(benchmark::State& state) {
    sd_e2::ParticlesAoS data;
    data.resize(static_cast<size_t>(state.range(0)));

    for (size_t i = 0; i < data.particles.size(); ++i) {
        data.particles[i].px = 0.1 * i;
        data.particles[i].py = 0.2 * i;
        data.particles[i].pz = 0.3 * i;
        data.particles[i].mass = 0.511;
    }

    for (auto _ : state) {
        double sum = data.sumEnergy();
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(BM_AoS_SumEnergy)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_SoA_SumEnergy(benchmark::State& state) {
    sd_e2::ParticlesSoA data;
    data.resize(static_cast<size_t>(state.range(0)));

    for (size_t i = 0; i < data.px.size(); ++i) {
        data.px[i] = 0.1 * i;
        data.py[i] = 0.2 * i;
        data.pz[i] = 0.3 * i;
        data.mass[i] = 0.511;
    }

    for (auto _ : state) {
        double sum = data.sumEnergy();
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(BM_SoA_SumEnergy)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();

