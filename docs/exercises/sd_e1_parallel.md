# SD-E1 Parallel Event Processing (OpenMP)

Goal: make a parallel loop **correct** first, then look at a common performance pitfall (**false sharing**).

You are given a serial reference implementation (`processEvent`) and a deliberately incorrect parallel implementation (`processEvents`).

## Success criteria

- With OpenMP enabled, `processEvents()` produces the same totals as the serial reference
- `ctest` passes
- You can explain the difference between a **race condition** and **false sharing**

## Where to work

```bash
cd exercises/SD-E1-parallel-event-processing/starter
```

## 0. Build + run the serial baseline

This is your correctness reference.

```bash
cmake -B build-serial -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-serial -j"$(nproc)"

./build-serial/event_processor
```

Note the printed totals. The OpenMP build should match.

## 1. Build + run with OpenMP

```bash
cmake -B build-omp -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_OPENMP=ON
cmake --build build-omp -j"$(nproc)"
```

The first configure may download Catch2 for the unit tests.

If you are offline, you can still build the executable with `-DBUILD_TESTING=OFF`, but you will not have `ctest` for correctness.

Run the tests:

```bash
ctest --test-dir build-omp --output-on-failure
```

If you run the executable directly, force more than one thread so the race shows up:

```bash
OMP_NUM_THREADS=4 ./build-omp/event_processor
```

Expected behaviour in the buggy starter:

- The OpenMP version can print **wrong totals** (nondeterministic)
- The unit test may fail intermittently, depending on thread scheduling

## 2. Find the race

Open:

- `src/EventProcessor.cpp`

In `processEvents`, two shared variables are updated concurrently:

```cpp
int tracks = 0;
double energy = 0.0;

#pragma omp parallel for
for (...) {
    ...
    tracks++;
    energy += particle.energy();
}
```

That is a data race. Multiple threads read-modify-write the same memory.

## 3. Fix correctness with an OpenMP reduction

The simplest fix is an OpenMP reduction. Each thread gets a private copy, then OpenMP combines them at the end.

Edit:

- `src/EventProcessor.cpp`

Patch:

```diff
--- a/src/EventProcessor.cpp
+++ b/src/EventProcessor.cpp
@@
 #ifdef CSC2026_USE_OPENMP
-#pragma omp parallel for
+#pragma omp parallel for reduction(+:tracks, energy)
 #endif
     for (size_t i = 0; i < events.size(); ++i) {
         for (const auto& particle : events[i].particles) {
-            tracks++;
-            energy += particle.energy();
+            tracks += 1;
+            energy += particle.energy();
         }
     }
```

Rebuild and re-run tests:

```bash
cmake --build build-omp -j"$(nproc)"
ctest --test-dir build-omp --output-on-failure
```

Then compare serial vs OpenMP totals:

```bash
./build-serial/event_processor
OMP_NUM_THREADS=4 ./build-omp/event_processor
```

The totals should match within floating-point tolerance.

## 4. Measure speedup

Use the provided timing output in `event_processor`.

```bash
for t in 1 2 4 8; do
  echo
  echo "Threads: $t"
  OMP_NUM_THREADS=$t ./build-omp/event_processor
done
```

You should see time drop as threads increase, up to a point.

## 5. Stretch: demonstrate false sharing (optional)

False sharing is a **performance** problem, not a correctness problem.

It happens when different threads update different variables that live on the **same cache line**. Cache coherence traffic slows everything down.

One way to demonstrate it is manual per-thread accumulation.

### Step 5.1: per-thread totals (correct but can be slow)

Replace the reduction with a per-thread array:

```cpp
struct Totals { int tracks; double energy; };
std::vector<Totals> totals(omp_get_max_threads(), Totals{0, 0.0});

#pragma omp parallel
{
  const int tid = omp_get_thread_num();
  #pragma omp for
  for (size_t i = 0; i < events.size(); ++i) {
    for (const auto& p : events[i].particles) {
      totals[tid].tracks += 1;
      totals[tid].energy += p.energy();
    }
  }
}

for (const auto& t : totals) {
  tracks += t.tracks;
  energy += t.energy;
}
```

If the per-thread structs are tightly packed, you may see worse performance than reduction due to false sharing.

### Step 5.2: fix false sharing with cache-line alignment

Pad/align the struct so each thread writes to a different cache line:

```cpp
struct alignas(64) Totals { int tracks; double energy; };
```

Re-run the timing loop and compare.

## Stretch

- Try ThreadSanitizer on the buggy version (races become explicit diagnostics).
- Compare `schedule(static)` vs `schedule(dynamic)` (work distribution vs overhead). 
