# TT-E1 Debugging with Sanitizers

Goal: use sanitizers to turn undefined behaviour into a precise file:line report, then fix the root cause and lock it with a test.

You will fix three real bug classes:

- **Heap buffer overflow** (off-by-one)
- **Use-after-free** (dangling pointer)
- **Memory leak** (missing `delete[]`)

## Success criteria

- `./analyze` runs with **no ASan/UBSan findings**
- `ctest` passes in the sanitizer build directory
- You add at least one regression test that would have caught one of the bugs

## Where to work

```bash
cd exercises/TT-E1-debugging-sanitizers/starter
```

## 0. Optional warm-up: run without sanitizers

This is here to show the problem: undefined behaviour can look "fine".

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"

./build/analyze
```

Do not trust the result. It may crash, or it may print numbers and exit.

## 1. Build with AddressSanitizer and UBSan

The first configure may download Catch2 for the unit tests.

If you are in a restricted/offline environment:

- You can configure with `-DBUILD_TESTING=OFF` to build the `analyze` executable without downloading Catch2.
- You will not be able to run `ctest` without Catch2.

### Recommended: Clang

```bash
rm -rf build-asan
CC=clang CXX=clang++ cmake -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build-asan -j"$(nproc)"
```

### Also OK: GCC

```bash
rm -rf build-asan
CC=gcc CXX=g++ cmake -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build-asan -j"$(nproc)"
```

Run the buggy program under sanitizers:

```bash
ASAN_OPTIONS=detect_leaks=1 ./build-asan/analyze
```

Sanitizers stop at the **first** detected issue. Fix one bug, rebuild, re-run.

## 2. Read the sanitizer report

What to look for in the output:

1. The **bug class** near the top, e.g. `heap-buffer-overflow`.
2. The **first stack frame in your code** (not in libc or libstdc++).
3. The **file and line number**. That is your starting point.

In this starter, the first failure should point into:

- `src/TrackReconstructor.cpp` in `TrackReconstructor::reconstruct()`

## 3. Fix 1: heap buffer overflow in `reconstruct()`

Open:

- `exercises/TT-E1-debugging-sanitizers/starter/src/TrackReconstructor.cpp`

The bug is an off-by-one loop bound (`<=` instead of `<`) while copying into a heap buffer.

Minimal fix:

```diff
--- a/src/TrackReconstructor.cpp
+++ b/src/TrackReconstructor.cpp
@@
-    for (size_t i = 0; i <= m_hits.size(); ++i) {
+    for (size_t i = 0; i < m_hits.size(); ++i) {
         hitBuffer[i] = m_hits[i];
     }
```

Rebuild and re-run:

```bash
cmake --build build-asan -j"$(nproc)"
ASAN_OPTIONS=detect_leaks=1 ./build-asan/analyze
```

You should now hit the next bug.

## 4. Fix 2: use-after-free in `getBestTrack()`

The current code allocates a `Track`, deletes it, then returns the pointer. That pointer is dangling.

Recommended fix: **return by value**.

### Step 4.1: change the API in the header

Edit:

- `include/TrackReconstructor.hpp`

```diff
--- a/include/TrackReconstructor.hpp
+++ b/include/TrackReconstructor.hpp
@@
-    // Intentionally buggy: returns pointer to freed memory (use-after-free)
-    const Track* getBestTrack() const;
+    // Return by value: avoids lifetime/ownership bugs
+    Track getBestTrack() const;
```

### Step 4.2: update the implementation

Edit:

- `src/TrackReconstructor.cpp`

```diff
--- a/src/TrackReconstructor.cpp
+++ b/src/TrackReconstructor.cpp
@@
-const Track* TrackReconstructor::getBestTrack() const {
-    auto* best = new Track{};
-    best->pt = 100.0;
-    best->hits = m_hits;
-
-    delete best;   // freed here
-    return best;   // ERROR: returning freed pointer
-}
+Track TrackReconstructor::getBestTrack() const {
+    Track best{};
+    best.pt = 100.0;
+    best.hits = m_hits;
+    return best;
+}
```

### Step 4.3: update the call site in `analyze.cpp`

Edit:

- `src/analyze.cpp`

```diff
--- a/src/analyze.cpp
+++ b/src/analyze.cpp
@@
-    const tt_e1::Track* best = reco.getBestTrack();
-    std::cout << "Best track pT: " << best->pt << std::endl;
+    const tt_e1::Track best = reco.getBestTrack();
+    std::cout << "Best track pT: " << best.pt << std::endl;
```

Rebuild and re-run:

```bash
cmake --build build-asan -j"$(nproc)"
ASAN_OPTIONS=detect_leaks=1 ./build-asan/analyze
```

If the use-after-free is fixed, the program should run further and exit normally.

## 5. Fix 3: memory leak in `reconstruct()`

`reconstruct()` allocates `hitBuffer` with `new[]` and never frees it.

Minimal fix: `delete[]` before returning.

```diff
--- a/src/TrackReconstructor.cpp
+++ b/src/TrackReconstructor.cpp
@@
     // Missing: delete[] hitBuffer;

+    delete[] hitBuffer;
     return tracks;
```

Rebuild and re-run:

```bash
cmake --build build-asan -j"$(nproc)"
ASAN_OPTIONS=detect_leaks=1 ./build-asan/analyze
```

At this point you should see **no sanitizer findings**.

## 6. Make tests catch the bugs

Run the tests:

```bash
ctest --test-dir build-asan --output-on-failure
```

The starter test only exercises `reconstruct()`. Add a small test to cover `getBestTrack()` too.

Edit:

- `tests/test_track_reconstructor.cpp`

Add a second test case:

```cpp
TEST_CASE("TrackReconstructor getBestTrack returns a valid Track") {
    tt_e1::TrackReconstructor reco(/*minPt=*/1.0);

    tt_e1::Hit h{};
    h.energy = 4.0;
    reco.addHit(h);

    const tt_e1::Track best = reco.getBestTrack();

    REQUIRE(best.pt > 0.0);
    REQUIRE(best.hits.size() == 1);
    REQUIRE(best.hits[0].energy == Catch::Approx(4.0));
}
```

Rebuild and re-run tests:

```bash
cmake --build build-asan -j"$(nproc)"
ctest --test-dir build-asan --output-on-failure
```

## Common pitfalls

- Leak reports only show up if the program **exits normally**. If it aborts earlier, fix the crash first.
- If you changed headers, make sure you rebuilt everything (a clean build directory is the simplest).
- Keep fixes small and re-run after each change. Sanitizers are most useful in tight iterations.

## Stretch

- Refactor `reconstruct()` to avoid manual `new[]/delete[]` entirely (RAII).
- Try `-DCMAKE_BUILD_TYPE=RelWithDebInfo` for a more realistic but still debuggable build.