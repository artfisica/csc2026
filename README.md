# CSC Latin America 2026 · HEP Computing Exercises (CERN / CSC)

High-Performance Computing patterns for HEP style analysis: correctness first, then parallelism, then performance.

**Course timetable / Indico:** https://indico.cern.ch/event/1551289/timetable/  

[![CI](https://github.com/artfisica/csc2026/actions/workflows/ci.yml/badge.svg)](https://github.com/artfisica/csc2026/actions/workflows/ci.yml)

---

## What this repository is

This repo is a **teaching codebase** used in lectures and demos, plus **self-contained exercise mini-projects** under `exercises/` (each one has its own `CMakeLists.txt` and a `starter/` folder).

It targets **modern C++ (C++17)**, ROOT-based workflows (via container/devcontainer), and a “measure + verify” mindset.

**Documentation:** https://artfisica.github.io/csc2026/

---

## Learning objectives

By the end of the course, students should be able to:

- Build reproducibly with CMake and Ninja
- Write small, meaningful unit tests (Catch2)
- Use sanitizers effectively:
  - ASan + UBSan for memory and UB
  - TSan for data races (with known limitations)
- Reason about parallelism costs (contention, false sharing, scheduling)
- Benchmark and optimize based on evidence, not guesses
- Use CI as a teaching guardrail (tests, sanitizers, docs)

---

## Quick start (recommended): GitHub Codespaces / Devcontainer

This repo ships a ready environment in `.devcontainer/` (ROOT base image + tooling).

- Codespaces link (repo must be public or you must be logged in):  
  `https://codespaces.new/artfisica/csc2026`

Once inside:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
````

Run the demos:

```bash
./build/analyze
./build/event_processor
```

---

## Local build (without Codespaces)

### Prerequisites

* CMake ≥ 3.16
* A recent C++ compiler (GCC/Clang)
* Ninja (recommended)
* ROOT available in your environment if you run the ROOT-dependent parts

### Build and test

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```

### Sanitizers

ASan + UBSan:

```bash
cmake -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build-asan -j"$(nproc)"
ctest --test-dir build-asan --output-on-failure
```

TSan:

```bash
cmake -B build-tsan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON
cmake --build build-tsan -j"$(nproc)"
ctest --test-dir build-tsan --output-on-failure
```

Note: OpenMP and TSan are often a bad mix. This repo’s CI treats TSan as a correctness check for race conditions, not a guarantee for OpenMP correctness.

---

## Exercises (the course core)

Each exercise is a **separate mini-project** with its own build. Always work inside the `starter/` folder unless instructed otherwise.

### TT-E1 · Debugging + Sanitizers

Path:

* `exercises/TT-E1-debugging-sanitizers/`

Focus:

* Reproduce crashes/UB
* Use ASan/UBSan to locate defects
* Add tests to prevent regressions

### SD-E1 · Parallel Event Processing (OpenMP + false sharing)

Path:

* `exercises/SD-E1-parallel-event-processing/`

Focus:

* Parallelize safely
* Spot false sharing and contention
* Measure scaling and identify bottlenecks

### SD-E2 · Benchmark and Optimize

Path:

* `exercises/SD-E2-benchmark-and-optimize/`

Focus:

* Use microbenchmarks correctly
* Compare layouts/approaches
* Optimize with evidence and keep correctness

### TT-E2 · Docs and CI

Path:

* `exercises/TT-E2-docs-and-ci/`

Focus:

* MkDocs workflow
* CI hygiene (fast feedback, reproducibility)
* Small, reviewable changes

---

## Docs (MkDocs)

The repo includes MkDocs config.

```bash
python3 -m pip install mkdocs mkdocs-material
mkdocs serve
```

---

## Repository layout

```
csc2026/
├── .devcontainer/          # Codespaces/DevContainer setup (ROOT image)
├── .github/workflows/      # CI pipelines (build/tests/sanitizers/bench/docs)
├── include/                # Public headers
├── src/                    # Main C++ implementation
├── tests/                  # Catch2 + Python tests
├── benchmarks/             # Microbenchmarks
├── exercises/              # Exercise mini-projects (each has starter/)
├── scripts/                # Utility scripts (e.g. sample ROOT generation)
├── docs/                   # Course docs and assets
└── CMakeLists.txt          # Top-level build
```

---

## Instructor notes

See:

* `INSTRUCTOR.md` for a teaching runbook
* `CONTRIBUTING.md` for expected workflows and sanity checks

---

## License / usage

Educational materials for CSC Latin America 2026. Reuse for teaching is fine. Keep attribution.

**Instructor:** Arturo Sánchez Pineda
GitHub: [https://github.com/artfisica](https://github.com/artfisica)
