# SD-E2 Benchmark and Optimise

Goal: measure first, then change one thing that improves performance **without breaking correctness**.

This mini-project benchmarks two data layouts:

- **AoS**: Array of Structures
- **SoA**: Structure of Arrays

The SoA version is intentionally written in a way that prevents good optimisation.

## Success criteria

- `bench_layout` builds and prints benchmark results
- You record **baseline** vs **improved** results
- You keep results correct (same numeric output within tolerance)

## Where to work

```bash
cd exercises/SD-E2-benchmark-and-optimize/starter
```

## 0. Build the starter

The first configure may download Google Benchmark if it is not already installed.

If you are offline and do not have a system install of Google Benchmark available, this project will not configure.
In that case, use a prepared course environment (Codespace/devcontainer) or ask the instructor for a fallback.

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

## 1. Run the baseline benchmark

Run a few repetitions and only print aggregates (easier to compare):

```bash
./build/bench_layout \
  --benchmark_format=console \
  --benchmark_repetitions=3 \
  --benchmark_report_aggregates_only=true
```

Write down the SoA vs AoS timings for each problem size.

Expected baseline behaviour:

- The **SoA** version can be slower than AoS because it uses expensive math (`std::pow`) in the hot loop.

## 2. Identify the hot code

Open:

- `include/ParticleLayouts.hpp`

Look at:

- `sd_e2::ParticlesSoA::sumEnergy()`

The inner loop uses `std::pow(x, 2)`. That is much more expensive than `x * x` and often blocks vectorisation.

## 3. Optimise one thing: remove `std::pow(x, 2)`

Replace the `pow` calls with plain multiplication.

Patch:

```diff
--- a/include/ParticleLayouts.hpp
+++ b/include/ParticleLayouts.hpp
@@
     double sumEnergy() const {
         double sum = 0.0;
         for (size_t i = 0; i < px.size(); ++i) {
-            // Intentionally slow: std::pow is expensive and blocks vectorization.
-            const double p2 = std::pow(px[i], 2) + std::pow(py[i], 2) + std::pow(pz[i], 2);
-            sum += std::sqrt(p2 + std::pow(mass[i], 2));
+            // Cheaper and compiler-friendly: multiply instead of pow(..., 2)
+            const double px2 = px[i] * px[i];
+            const double py2 = py[i] * py[i];
+            const double pz2 = pz[i] * pz[i];
+            const double m2  = mass[i] * mass[i];
+
+            const double p2 = px2 + py2 + pz2;
+            sum += std::sqrt(p2 + m2);
         }
         return sum;
     }
```

Rebuild:

```bash
cmake --build build -j"$(nproc)"
```

## 4. Add a quick correctness check

Benchmarks should not be your only correctness signal.

Add a small executable that checks AoS and SoA compute the same sum (within tolerance).

### Step 4.1: add a new source file

Create `src/verify_layout.cpp`:

```cpp
#include "ParticleLayouts.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>

int main() {
    constexpr size_t N = 10000;

    sd_e2::ParticlesAoS aos;
    sd_e2::ParticlesSoA soa;
    aos.resize(N);
    soa.resize(N);

    for (size_t i = 0; i < N; ++i) {
        const double px = 0.1 * i;
        const double py = 0.2 * i;
        const double pz = 0.3 * i;
        const double m  = 0.511;

        aos.particles[i].px = px;
        aos.particles[i].py = py;
        aos.particles[i].pz = pz;
        aos.particles[i].mass = m;

        soa.px[i] = px;
        soa.py[i] = py;
        soa.pz[i] = pz;
        soa.mass[i] = m;
    }

    const double a = aos.sumEnergy();
    const double b = soa.sumEnergy();

    const double rel = std::abs(a - b) / (std::abs(a) + 1e-12);
    std::cout << "AoS sum: " << a << "\n";
    std::cout << "SoA sum: " << b << "\n";
    std::cout << "Relative diff: " << rel << "\n";

    if (rel > 1e-12) {
        std::cerr << "Mismatch: optimisation changed the result too much\n";
        return 1;
    }

    return 0;
}
```

### Step 4.2: wire it into CMake

Edit `CMakeLists.txt`:

```diff
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@
 add_executable(bench_layout
   src/bench_layout.cpp
 )
 target_include_directories(bench_layout PRIVATE include)
 target_link_libraries(bench_layout PRIVATE benchmark::benchmark)

+add_executable(verify_layout
+  src/verify_layout.cpp
+)
+target_include_directories(verify_layout PRIVATE include)
```

Rebuild and run the check:

```bash
cmake --build build -j"$(nproc)"
./build/verify_layout
```

## 5. Re-run the benchmark and compare

```bash
./build/bench_layout \
  --benchmark_format=console \
  --benchmark_repetitions=3 \
  --benchmark_report_aggregates_only=true
```

Compare against the baseline numbers you wrote down. You should see SoA improve significantly.

## Stretch

- Try `RelWithDebInfo` and inspect the generated assembly (`-S`) to see vectorisation.
- Add one more benchmark size (e.g. `->Arg(1000000)`) and see if the crossover point changes.
- Explore compiler flags like `-march=native` (only when comparing on the same machine).