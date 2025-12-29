# SD-E2 Starter

This project contains a simple benchmark to optimise.

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run benchmark

```bash
./build/bench_layout
```

## What to optimise

- Replace expensive math (`pow`) with cheaper equivalents
- Improve loops to help the compiler vectorise
- Consider data layout and cache friendliness

