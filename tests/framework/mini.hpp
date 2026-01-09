
#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <utility>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <iomanip>    // for std::setprecision
#include <algorithm>  // for std::max

namespace mtest {

// ---------- Config ----------
#ifndef MTEST_ENABLE_EXCEPTIONS
#define MTEST_ENABLE_EXCEPTIONS 1
#endif

#ifndef MTEST_DEFAULT_OUTPUT_TAP
#define MTEST_DEFAULT_OUTPUT_TAP 0
#endif

#ifndef MTEST_DEFAULT_THREADS
#define MTEST_DEFAULT_THREADS 1
#endif

// ---------- Core types ----------
struct TestContext {
    const char* suite = nullptr;
    std::string name;               // dynamic instance names supported
    const char* tag   = nullptr;    // "unit", "module", "usecase", …
    const char* file  = nullptr;
    int         line  = 0;
    std::string message;
    bool        failed = false;
};

using testfn = std::function<void(TestContext&)>;

struct TestCase {
    const char* suite;
    std::string name;   // allow dynamic instance names
    const char* tag;
    const char* file;
    int         line;
    testfn      fn;
};

// ---------- Registry ----------
inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

struct Registrar {
    Registrar(const char* suite, const char* name, const char* tag,
              const char* file, int line, testfn fn) {
        registry().push_back({suite, std::string(name), tag, file, line, std::move(fn)});
    }
    // Overload for std::string names (parameterized cases)
    Registrar(const char* suite, const std::string& name, const char* tag,
              const char* file, int line, testfn fn) {
        registry().push_back({suite, name, tag, file, line, std::move(fn)});
    }
};

// ---------- Utilities ----------
namespace detail {
    // C++17-friendly to_string_any
    template <typename T>
    inline std::string to_string_any(const T& v) {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }

    // ANSI colors
    struct Colors {
        const char* reset  = "\x1b[0m";
        const char* bold   = "\x1b[1m";
        const char* red    = "\x1b[31m";
        const char* green  = "\x1b[32m";
        const char* yellow = "\x1b[33m";
        const char* cyan   = "\x1b[36m";
    };

    inline bool env_no_color() {
        const char* nc = std::getenv("NO_COLOR");
        return nc && *nc;
    }

    inline std::string fmt_pass(const TestCase& tc, double ms, bool tap, bool color) {
        Colors c;
        std::ostringstream oss;
        if (tap) {
            oss << "ok - " << tc.suite << "." << tc.name << " [" << tc.tag << "] ("
                << std::fixed << std::setprecision(3) << ms << " ms)\n";
        } else {
            if (color) oss << c.green;
            oss << "[PASS] ";
            if (color) oss << c.reset;
            oss << tc.suite << "." << tc.name << " [" << tc.tag << "] ("
                << std::fixed << std::setprecision(3) << ms << " ms)\n";
        }
        return oss.str();
    }

    inline std::string fmt_fail(const TestCase& tc, double ms, const TestContext& ctx,
                                bool tap, bool color) {
        Colors c;
        std::ostringstream oss;
        if (tap) {
            oss << "not ok - " << tc.suite << "." << tc.name << " [" << tc.tag << "] ("
                << std::fixed << std::setprecision(3) << ms << " ms)\n";
            oss << "  ---\n  file: " << tc.file << "\n  line: " << tc.line
                << "\n  msg: " << ctx.message << "\n  ...\n";
        } else {
            if (color) oss << c.red;
            oss << "[FAIL] ";
            if (color) oss << c.reset;
            oss << tc.suite << "." << tc.name << " [" << tc.tag << "] ("
                << std::fixed << std::setprecision(3) << ms << " ms)\n  "
                << tc.file << ":" << tc.line << "\n  " << ctx.message << "\n";
        }
        return oss.str();
    }
}

// ---------- Assertions ----------
#define MTEST_STRINGIFY(x) #x
#define MTEST_STR(x) MTEST_STRINGIFY(x)

#define MT_REQUIRE(ctx, expr) do { \
    if (!(expr)) { \
        (ctx).failed = true; \
        (ctx).message = std::string("REQUIRE failed: ") + #expr; \
        if (MTEST_ENABLE_EXCEPTIONS) throw std::runtime_error((ctx).message); \
        return; \
    } \
} while(0)

#define MT_CHECK(ctx, expr) do { \
    if (!(expr)) { \
        (ctx).failed = true; \
        (ctx).message = std::string("CHECK failed: ") + #expr; \
    } \
} while(0)

#define MT_REQUIRE_EQ(ctx, a, b) do { \
    auto va = (a); auto vb = (b); \
    if (!(va == vb)) { \
        (ctx).failed = true; \
        std::ostringstream _m; \
        _m << "REQUIRE_EQ failed: " << #a << " == " << #b \
            << "  (got " << va << " vs " << vb << ")"; \
        (ctx).message = _m.str(); \
        if (MTEST_ENABLE_EXCEPTIONS) throw std::runtime_error((ctx).message); \
        return; \
    } \
} while(0)

#define MT_CHECK_EQ(ctx, a, b) do { \
    auto va = (a); auto vb = (b); \
    if (!(va == vb)) { \
        (ctx).failed = true; \
        std::ostringstream _m; \
        _m << "CHECK_EQ failed: " << #a << " == " << #b \
            << "  (got " << va << " vs " << vb << ")"; \
        (ctx).message = _m.str(); \
    } \
} while(0)

#define MT_FAIL(ctx, msg) do { \
    (ctx).failed = true; \
    (ctx).message = std::string("FAIL: ") + (msg); \
    if (MTEST_ENABLE_EXCEPTIONS) throw std::runtime_error((ctx).message); \
    return; \
} while(0)

// ---------- Fixtures ----------
struct Fixture {
    virtual ~Fixture() = default;
    virtual void setup(TestContext&) {}
    virtual void teardown(TestContext&) {}
};

template <typename F>
inline void with_fixture(TestContext& ctx,
    const std::function<void(F&, TestContext&)>& body) {
    F f;
    f.setup(ctx);
    body(f, ctx);
    f.teardown(ctx);
}

// ---------- Registration macros ----------
#define MT_TEST(suite, name, tag) \
    static void suite##_##name##_impl(mtest::TestContext&); \
    static mtest::Registrar suite##_##name##_reg( \
        MTEST_STRINGIFY(suite), MTEST_STRINGIFY(name), tag, \
        __FILE__, __LINE__, suite##_##name##_impl \
    ); \
    static void suite##_##name##_impl(mtest::TestContext& ctx)

// Lambda-free fixture macro: define body separately and call via std::bind.
#define MT_TEST_F(suite, name, tag, FixtureType) \
    static void suite##_##name##_body(FixtureType&, mtest::TestContext&); \
    static void suite##_##name##_impl(mtest::TestContext&); \
    static mtest::Registrar suite##_##name##_reg( \
        MTEST_STRINGIFY(suite), MTEST_STRINGIFY(name), tag, \
        __FILE__, __LINE__, suite##_##name##_impl \
    ); \
    static void suite##_##name##_impl(mtest::TestContext& ctx) { \
        mtest::with_fixture<FixtureType>(ctx, std::bind( \
            suite##_##name##_body, std::placeholders::_1, std::placeholders::_2)); \
    } \
    static void suite##_##name##_body(FixtureType& f, mtest::TestContext& ctx)

// (Optional) define MT_TEST_F_END as a no-op for compatibility
#define MT_TEST_F_END

//
// Parameterized tests by value.
// Example:
//   MT_TEST_P_LIST(math, add_param, "unit", int, 1, 2, 3) { /* use `param` */ }
// Or:
//   MT_TEST_P(math, add_param, "unit", int,
//             (std::initializer_list<int>{1,2,3})) { /* use `param` */ }
//
#define MT_TEST_P(suite, name, tag, ParamType, params) \
    static void suite##_##name##_impl(mtest::TestContext&, const ParamType& param); \
    namespace suite##_##name##_param_ns { \
        static struct registrar_t { \
            registrar_t() { \
                for (const ParamType& _p : params) { \
                    std::string _n = std::string(MTEST_STRINGIFY(name)) + "(" + mtest::detail::to_string_any(_p) + ")"; \
                    mtest::Registrar( \
                        MTEST_STRINGIFY(suite), _n, tag, __FILE__, __LINE__, \
                        std::bind(suite##_##name##_impl, std::placeholders::_1, _p) \
                    ); \
                } \
            } \
        } _registrar_instance; \
    } \
    static void suite##_##name##_impl(mtest::TestContext& ctx, const ParamType& param)

// Varargs-friendly version to avoid comma parsing (preferred)
#define MT_TEST_P_LIST(suite, name, tag, ParamType, ...) \
    static void suite##_##name##_impl(mtest::TestContext&, const ParamType& param); \
    namespace suite##_##name##_param_list_ns { \
        static struct registrar_t { \
            registrar_t() { \
                const ParamType _params[] = { __VA_ARGS__ }; \
                for (const ParamType& _p : _params) { \
                    std::string _n = std::string(MTEST_STRINGIFY(name)) + "(" + mtest::detail::to_string_any(_p) + ")"; \
                    mtest::Registrar( \
                        MTEST_STRINGIFY(suite), _n, tag, __FILE__, __LINE__, \
                        std::bind(suite##_##name##_impl, std::placeholders::_1, _p) \
                    ); \
                } \
            } \
        } _registrar_instance; \
    } \
    static void suite##_##name##_impl(mtest::TestContext& ctx, const ParamType& param)

// Fixture + parameterized (no lambdas)
#define MT_TEST_FP(suite, name, tag, FixtureType, ParamType, params) \
    static void suite##_##name##_body(FixtureType&, mtest::TestContext&, const ParamType&); \
    static void suite##_##name##_impl(mtest::TestContext&); \
    namespace suite##_##name##_fp_ns { \
        static struct registrar_t { \
            registrar_t() { \
                for (const ParamType& _p : params) { \
                    std::string _n = std::string(MTEST_STRINGIFY(name)) + "(" + mtest::detail::to_string_any(_p) + ")"; \
                    mtest::Registrar( \
                        MTEST_STRINGIFY(suite), _n, tag, __FILE__, __LINE__, \
                        std::bind(suite##_##name##_impl, std::placeholders::_1) \
                    ); \
                } \
            } \
        } _registrar_instance; \
    } \
    static void suite##_##name##_impl(mtest::TestContext& ctx) { \
        /* We cannot carry param via Registrar with no lambdas; \
           so parameterized + fixture requires MT_TEST_FP_LIST (below). */ \
        (void)ctx; \
    } \
    static void suite##_##name##_body(FixtureType& f, mtest::TestContext& ctx, const ParamType& param)

// Preferred: Fixture + parameterized with varargs (each instance registered)
#define MT_TEST_FP_LIST(suite, name, tag, FixtureType, ParamType, ...) \
    static void suite##_##name##_body(FixtureType&, mtest::TestContext&, const ParamType&); \
    static void suite##_##name##_impl_instance(mtest::TestContext&, const ParamType&); \
    namespace suite##_##name##_fp_list_ns { \
        static struct registrar_t { \
            registrar_t() { \
                const ParamType _params[] = { __VA_ARGS__ }; \
                for (const ParamType& _p : _params) { \
                    std::string _n = std::string(MTEST_STRINGIFY(name)) + "(" + mtest::detail::to_string_any(_p) + ")"; \
                    mtest::Registrar( \
                        MTEST_STRINGIFY(suite), _n, tag, __FILE__, __LINE__, \
                        std::bind(suite##_##name##_impl_instance, std::placeholders::_1, _p) \
                    ); \
                } \
            } \
        } _registrar_instance; \
    } \
    static void suite##_##name##_impl_instance(mtest::TestContext& ctx, const ParamType& param) { \
        mtest::with_fixture<FixtureType>(ctx, std::bind( \
            suite##_##name##_body, std::placeholders::_1, std::placeholders::_2, param)); \
    } \
    static void suite##_##name##_body(FixtureType& f, mtest::TestContext& ctx, const ParamType& param)

// ---------- Runner ----------
struct RunOptions {
    const char* name_contains = nullptr;   // substring filter on test name
    const char* tag_equals    = nullptr;   // exact tag filter
    bool        tap_output    = MTEST_DEFAULT_OUTPUT_TAP != 0;
    bool        stop_on_fail  = false;
    bool        summary_only  = false;
    bool        color_output  = true;      // colored output
    int         threads       = MTEST_DEFAULT_THREADS; // parallelism
};

struct RunResult {
    int total   = 0;
    int failed  = 0;
    int passed  = 0;
    double ms_total = 0.0;
};

inline bool match_filter(const TestCase& tc, const RunOptions& opt) {
    if (opt.name_contains) {
        if (tc.name.find(opt.name_contains) == std::string::npos)
            return false;
    }
    if (opt.tag_equals) {
        if (std::strcmp(tc.tag, opt.tag_equals) != 0)
            return false;
    }
    return true;
}

struct CaseResult {
    bool failed = false;
    double ms   = 0.0;
    std::string out;
    std::string msg;
};

inline CaseResult run_one(const TestCase& tc, const RunOptions& opt, int tap_index) {
    CaseResult cr;
    TestContext ctx;
    ctx.suite = tc.suite; ctx.name = tc.name; ctx.tag = tc.tag;
    ctx.file  = tc.file;  ctx.line = tc.line;

    auto t0 = std::chrono::steady_clock::now();
#if MTEST_ENABLE_EXCEPTIONS
    try {
        tc.fn(ctx);
    } catch (const std::exception& e) {
        ctx.failed = true;
        ctx.message = std::string("uncaught: ") + e.what();
    } catch (...) {
        ctx.failed = true;
        ctx.message = "uncaught: unknown exception";
    }
#else
    tc.fn(ctx);
#endif
    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    cr.failed = ctx.failed;
    cr.ms     = ms;
    cr.msg    = ctx.message;

    // Format output (TAP includes index prefix)
    if (opt.tap_output) {
        std::ostringstream oss;
        if (!ctx.failed) {
            oss << "ok " << tap_index << " - " << tc.suite << "." << tc.name
                << " [" << tc.tag << "] (" << std::fixed << std::setprecision(3) << ms << " ms)\n";
        } else {
            oss << "not ok " << tap_index << " - " << tc.suite << "." << tc.name
                << " [" << tc.tag << "] (" << std::fixed << std::setprecision(3) << ms << " ms)\n";
            oss << "  ---\n  file: " << tc.file << "\n  line: " << tc.line
                << "\n  msg: " << ctx.message << "\n  ...\n";
        }
        cr.out = oss.str();
    } else {
        cr.out = ctx.failed
            ? detail::fmt_fail(tc, ms, ctx, /*tap*/false, opt.color_output && !detail::env_no_color())
            : detail::fmt_pass(tc, ms, /*tap*/false, opt.color_output && !detail::env_no_color());
    }

    return cr;
}

// Worker functor for parallel execution (no lambdas)
struct Worker {
    const std::vector<int>* selected;
    const std::vector<TestCase>* cases;
    const RunOptions* opt;
    std::vector<CaseResult>* results;
    std::atomic<size_t>* next_index;

    Worker(const std::vector<int>* sel,
           const std::vector<TestCase>* rcases,
           const RunOptions* ropt,
           std::vector<CaseResult>* res,
           std::atomic<size_t>* next)
        : selected(sel), cases(rcases), opt(ropt), results(res), next_index(next) {}

    void operator()() {
        while (true) {
            size_t k = next_index->fetch_add(1);
            if (k >= selected->size()) break;
            int idx = (*selected)[k];
            (*results)[k] = run_one((*cases)[idx], *opt, (int)k + 1);
        }
    }
};

inline RunResult run_all(const RunOptions& opt = {}) {
    RunResult rr{};
    const auto& r = registry();

    // Collect selected cases
    std::vector<int> selected;
    selected.reserve(r.size());
    for (int i = 0; i < (int)r.size(); ++i) {
        if (match_filter(r[i], opt)) selected.push_back(i);
    }

    if (opt.tap_output) {
        std::printf("TAP version 13\n");
        std::printf("1..%zu\n", selected.size());
    }

    // Results store in deterministic order
    std::vector<CaseResult> results(selected.size());

    // If stop_on_fail or threads <= 1, run sequential
    if (opt.stop_on_fail || opt.threads <= 1) {
        for (size_t k = 0; k < selected.size(); ++k) {
            int idx = selected[k];
            auto cr = run_one(r[idx], opt, (int)k + 1);
            results[k] = std::move(cr);
            rr.total++;
            rr.ms_total += results[k].ms;
            if (!results[k].failed) rr.passed++;
            else { rr.failed++; if (opt.stop_on_fail) break; }
            if (!opt.summary_only) {
                std::fwrite(results[k].out.data(), 1, results[k].out.size(), stdout);
            }
        }
    } else {
        // Parallel execution with deterministic output ordering
        std::atomic<size_t> next{0};
        int threads = std::max(1, opt.threads);
        std::vector<std::thread> workers;
        workers.reserve(threads);

        for (int t = 0; t < threads; ++t) {
            workers.emplace_back(Worker(&selected, &r, &opt, &results, &next));
        }
        for (std::thread& th : workers) th.join();

        // Aggregate + print in order
        for (size_t k = 0; k < results.size(); ++k) {
            rr.total++;
            rr.ms_total += results[k].ms;
            if (!results[k].failed) rr.passed++; else rr.failed++;
            if (!opt.summary_only) {
                std::fwrite(results[k].out.data(), 1, results[k].out.size(), stdout);
            }
        }
    }

    if (!opt.summary_only) {
        bool all_ok = (rr.failed == 0);
        if (opt.color_output && !detail::env_no_color()) {
            const char* col = all_ok ? "\x1b[32m" : "\x1b[31m";
            std::printf("\n%sSummary:%s total=%d, passed=%d, failed=%d, time=%.3f ms\n",
                        col, "\x1b[0m", rr.total, rr.passed, rr.failed, rr.ms_total);
        } else {
            std::printf("\nSummary: total=%d, passed=%d, failed=%d, time=%.3f ms\n",
                        rr.total, rr.passed, rr.failed, rr.ms_total);
        }
    }
    return rr;
}

// Namespaced runner (not the global entry point)
inline int run_main(int argc, const char* argv[]) {
    RunOptions opt{};
    for (int i=1; i<argc; ++i) {
        const char* a = argv[i];
        if (std::strncmp(a, "--filter=", 9) == 0) opt.name_contains = a+9;
        else if (std::strncmp(a, "--tag=", 6) == 0) opt.tag_equals = a+6;
        else if (std::strcmp(a, "--tap") == 0) opt.tap_output = true;
        else if (std::strcmp(a, "--stop-on-fail") == 0) opt.stop_on_fail = true;
        else if (std::strcmp(a, "--summary") == 0) opt.summary_only = true;
        else if (std::strcmp(a, "--no-tap") == 0) opt.tap_output = false;
        else if (std::strcmp(a, "--no-color") == 0) opt.color_output = false;
        else if (std::strcmp(a, "--color") == 0) opt.color_output = true;
        else if (std::strncmp(a, "--threads=", 10) == 0) {
            int v = std::atoi(a+10);
            if (v > 0) opt.threads = v;
        }
    }
    auto rr = run_all(opt);
    return rr.failed ? 1 : 0;
}

} // namespace mtest

// Global main wrapper (can be disabled with -DMTEST_NO_MAIN)
#ifndef MTEST_NO_MAIN
int main(int argc, const char* argv[]) {
    return mtest::run_main(argc, argv);
}
#endif

