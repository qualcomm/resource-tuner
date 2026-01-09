
# Mini Test Framework (`mtest`)

A lightweight, header-only C++ testing framework designed for simplicity, speed, and flexibility. It supports unit tests, component tests, fixtures, parameterized tests, parallel execution, TAP output, filtering, colored output, and concise summaries.

---

## Table of Contents
- [Features](#features)
- [Quick Start](#quick-start)
- [Writing Tests](#writing-tests)
  - [Basic Test](#basic-test)
  - [Assertions](#assertions)
  - [Fixture-Based Tests](#fixture-based-tests)
  - [Parameterized Tests](#parameterized-tests)
  - [Fixture + Parameterized](#fixture--parameterized)
- [Running Tests](#running-tests)
  - [CLI Options](#cli-options)
  - [Exit Codes](#exit-codes)
  - [Output Examples](#output-examples)
- [Build & Install](#build--install)
- [Advanced Usage](#advanced-usage)
  - [Parallel Execution](#parallel-execution)
  - [TAP Output](#tap-output)
  - [Color Control](#color-control)
  - [Disabling Global `main`](#disabling-global-main)
  - [Exception Handling](#exception-handling)
- [CMake Integration](#cmake-integration)
- [Design Notes](#design-notes)
- [FAQ](#faq)

---

## Features
-  **Header-only**: single include (`mini.hpp`).
-  **Simple test definition macros** (`MT_TEST`, `MT_TEST_F`, `MT_TEST_P_LIST`, `MT_TEST_FP_LIST`).
-  **Assertions**: `MT_REQUIRE`, `MT_CHECK`, `MT_REQUIRE_EQ`, `MT_CHECK_EQ`, `MT_FAIL`.
-  **Fixtures** with `setup/teardown` lifecycle.
-  **Parameterized tests** without lambdas.
-  **Parallel execution** with deterministic output ordering.
-  **Filtering** by test name substring and exact tag.
-  **TAP v13 output** (for CI integration) and colored human-readable output.
-  **Timing** per test and total summary.
-  **Optional global `main`** (disable with `-DMTEST_NO_MAIN`).

---

## Quick Start
1. **Include** the header in your test file:
   ```cpp
   #include "mini.hpp"
   ```
2. **Write a test**:
   ```cpp
   MT_TEST(math, addition, "unit") {
       int a = 2, b = 3;
       MT_REQUIRE_EQ(ctx, a + b, 5);
   }
   ```
3. **Build**:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_INSTALL_PREFIX=/ -DBUILD_TESTS=ON
   make
   ```
4. **Run**:
   ```bash
   ./resource-tuner/tests/RestuneMiniTests
   ```

---

## Writing Tests

### Basic Test
```cpp
MT_TEST(math, addition, "unit") {
    int a = 2, b = 3;
    MT_REQUIRE(ctx, (a + b) == 5);
}
```
- `suite`: logical group (e.g., `math`).
- `name`: test name (e.g., `addition`).
- `tag`: category (`unit`, `component`, `component-serial`, `integration`, etc.).

### Assertions
- `MT_REQUIRE(ctx, expr)` – mark failure and **stop** the test (throws if exceptions enabled).
- `MT_CHECK(ctx, expr)` – mark failure but **continue** the test.
- `MT_REQUIRE_EQ(ctx, a, b)` – equality check; **stop** on mismatch (with detailed message).
- `MT_CHECK_EQ(ctx, a, b)` – equality check; **continue** on mismatch.
- `MT_FAIL(ctx, "message")` – explicit failure and **stop** the test.

All assertion macros populate `ctx.message` and set `ctx.failed=true`. When `MTEST_ENABLE_EXCEPTIONS` is non-zero (default **1**), `REQUIRE`/`FAIL` throw `std::runtime_error` to unwind quickly.

### Fixture-Based Tests
```cpp
struct MyFixture : mtest::Fixture {
    int value = 0;
    void setup(mtest::TestContext&) override { value = 42; }
    void teardown(mtest::TestContext&) override { /* cleanup if needed */ }
};

MT_TEST_F(math, fixture_test, "unit", MyFixture) {
    MT_REQUIRE_EQ(ctx, f.value, 42);
}
MT_TEST_F_END
```
- `setup` runs before the body; `teardown` after the body.
- Fixture instance is constructed per test case for isolation.

### Parameterized Tests
Use `MT_TEST_P_LIST` (preferred) to avoid complex macro comma parsing:
```cpp
MT_TEST_P_LIST(math, add_param, "unit", int, 1, 2, 3) {
    // `param` is the current value
    MT_CHECK(ctx, param > 0);
}
```
Or `MT_TEST_P` with an initializer list:
```cpp
MT_TEST_P(math, add_param2, "unit", int, (std::initializer_list<int>{4,5,6})) {
    MT_REQUIRE(ctx, param >= 4);
}
```
Each parameter registers a separate test instance with the name `name(value)`.

### Fixture + Parameterized
Use `MT_TEST_FP_LIST` to register each instance:
```cpp
struct Env : mtest::Fixture { /* ...setup... */ };

MT_TEST_FP_LIST(io, open_modes, "component", Env, int, 0, 1, 2) {
    // `param` available; fixture `f` provided
    MT_CHECK(ctx, param >= 0);
}
```

---

## Running Tests
Invoke your test binary directly (e.g., `RestuneMiniTests`).

### CLI Options
- `--filter=<substring>` – run cases whose **name contains** substring.
- `--tag=<tag>` – run cases matching **exact** tag.
- `--tap` / `--no-tap` – enable/disable TAP output (default off).
- `--stop-on-fail` – stop after the first failing test.
- `--summary` – only print summary lines.
- `--threads=N` – run in parallel with `N` worker threads (deterministic output order preserved).
- `--color` / `--no-color` – force/disable colored output.

### Exit Codes
- **0** – no failures.
- **1** – one or more failures occurred.

### Output Examples
Human-readable (default):
```
[PASS] math.addition [unit] (0.002 ms)
[FAIL] math.subtraction [unit] (0.001 ms)
  test.cpp:42
  REQUIRE failed: (a - b) == 1

Summary: total=2, passed=1, failed=1, time=0.003 ms
```

TAP v13 (`--tap`):
```
TAP version 13
1..2
ok 1 - math.addition [unit] (0.002 ms)
not ok 2 - math.subtraction [unit] (0.001 ms)
  ---
  file: test.cpp
  line: 42
  msg: REQUIRE failed: (a - b) == 1
  ...
```

---

## Build & Install
From your project root:
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/ -DBUILD_TESTS=ON
make
sudo make install
```
This will build and install:
- Test binaries (e.g., `/usr/bin/RestuneMiniTests`).
- Config/test data under `/etc/urm/tests/...` (if applicable to your project).
- Libraries under `/usr/lib` and headers under `/usr/include/Urm` (project-specific).

You can also run the in-tree binary:
```bash
./resource-tuner/tests/RestuneMiniTests
```

---

## Advanced Usage

### Parallel Execution
Use `--threads=N` to enable parallelism. The framework:
- Executes cases concurrently.
- Preserves **deterministic output order** by storing results and printing in registration order.

Example:
```bash
./resource-tuner/tests/RestuneMiniTests --threads=4
```

### TAP Output
Enable TAP with `--tap` to integrate with CI systems that consume TAP v13:
```bash
./resource-tuner/tests/RestuneMiniTests --tap
```
TAP lines include test indices, names, tags, timings, and failure diagnostics.

### Color Control
- By default, colored output is enabled.
- Set environment variable `NO_COLOR=1` to disable colors globally.
- Or pass `--no-color`/`--color` on the CLI.

### Disabling Global `main`
If you need to provide your own `main`, build with:
```bash
-DMTEST_NO_MAIN
```
Then call the runner directly:
```cpp
int main(int argc, const char* argv[]) {
    return mtest::run_main(argc, argv);
}
```

### Exception Handling
- Exceptions are **enabled** by default (`#define MTEST_ENABLE_EXCEPTIONS 1`).
- To compile without exception throws (assertions mark failure and return), define:
```cpp
#define MTEST_ENABLE_EXCEPTIONS 0
#include "mini.hpp"
```

---

## CMake Integration
Example CMake snippet to add a test target:
```cmake
# tests/CMakeLists.txt
add_executable(RestuneMiniTests
    tests/main.cpp        # or any file that includes mini.hpp
    tests/threadpool_tests.cpp
    tests/timer_tests.cpp
    tests/parser_tests.cpp
)

# If mini.hpp is header-only in the source tree
target_include_directories(RestuneMiniTests PRIVATE ${CMAKE_SOURCE_DIR}/resource-tuner/tests/framework)

# Link project libs as needed
target_link_libraries(RestuneMiniTests PRIVATE RestuneCore RestunePlugin RestuneTestUtils)

install(TARGETS RestuneMiniTests RUNTIME DESTINATION bin)
```

Run from build tree:
```bash
./resource-tuner/tests/RestuneMiniTests --filter=threadpool --tag=component-serial --threads=2
```

---

## Design Notes
- **Registration**: Each test case registers itself via a `Registrar` into a global registry at static init.
- **Context**: `TestContext` carries metadata and failure message, ensuring consistent reporting.
- **Determinism**: Parallel workers compute results in any order, but printing is serialized in registration order.
- **No lambdas in macros**: Avoids ODR and macro pitfalls; `std::bind` is used internally where necessary.

---

## FAQ

**Q: How can I restrict which tests run?**
- Use `--filter=<substring>` for name-based selection and `--tag=<tag>` for exact tag matches.

**Q: How do I integrate with CI?**
- Prefer `--tap` for TAP-compatible parsers or parse the summary lines. Exit code 1 indicates failure.

**Q: Can I turn off colors globally?**
- Yes: `export NO_COLOR=1` or pass `--no-color`.

**Q: Can I provide custom reporting?**
- Use `mtest::run_all(const RunOptions&)` directly and format `RunResult` yourself.

---

## Real Run Example
From latest run:
```
Summary: total=90, passed=90, failed=0, time=179527.907 ms
```
Demonstrating parallel-safe reporting, timing, and tags like `component-serial`, `unit`, etc.

---

