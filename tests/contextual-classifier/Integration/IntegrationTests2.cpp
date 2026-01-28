// test_classifier_integration.cpp
//
// Simple, well-documented integration test for the Contextual Classifier
// using the provided mini.hpp framework.
//
// What we assert:
//   1) URM loads the classifier module (InitTrace + Init logs).
//   2) Netlink listener is active (“Now listening for process events.”).
//   3) Launching a short-lived process (sleep 1) results in a
//      “ClassifyProcess: Starting classification for PID: …” log.
// We DO NOT require signals, fastText model, or cgroup slices to succeed.
// Any “Signal ID 0x0 not found” or cgroup write failures are ignored for this test.
//
// Build (example):
//   g++ -std=c++17 -O2 test_classifier_integration.cpp -o test_cc
//
// Run (if journal needs root):
//   sudo ./test_cc
//
// Optional filters:
//   ./test_cc --filter=Init
//   ./test_cc --filter=Event
//
// Reports (auto-emit test_cc.json / .junit.xml / .md in CWD):
//   env TEST_REPORT_DIR=./reports ./test_cc
//
// ---------------------------------------------------------------------

//#define MTEST_NO_MAIN 1
#include "../framework/mini.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <regex>

namespace {

// ---- Adjust if your service/tag strings differ ----
constexpr const char* kService   = "urm";
constexpr const char* kInitTag1  = "InitTrace: ENTER module init";
constexpr const char* kInitTag2  = "InitTrace: calling cc_init()";
constexpr const char* kInitTag3  = "Classifier module init.";
constexpr const char* kListenTag = "Now listening for process events.";
// Worker consumption marker used by ContextualClassifier.cpp:
static const std::regex kClassifyStartRe(
    R"(ClassifyProcess:\s*Starting classification for PID:\s*\d+)",
    std::regex::icase);

// How far back to query journal (safer than wall-clock ISO formatting).
// Keep tight to reduce noise, but long enough to include service restart.
constexpr int kSinceSeconds = 180;   // last 3 minutes
constexpr int kSettleMs     = 1800;  // settle time after restart
constexpr int kEventWaitMs  = 1500;  // time for netlink/worker to process sleep

// ---- Small helpers ---------------------------------------------------

struct CmdResult {
    int exit_code = -1;
    std::string out;
};

CmdResult run_cmd(const std::string& cmd) {
    CmdResult cr;
    std::array<char, 4096> buf{};
#ifdef _WIN32
    // Not supported here
    cr.exit_code = -1;
    return cr;
#else
    FILE* fp = popen((cmd + " 2>/dev/null").c_str(), "r");
    if (!fp) { cr.exit_code = -1; return cr; }
    while (size_t n = fread(buf.data(), 1, buf.size(), fp)) {
        cr.out.append(buf.data(), n);
    }
    cr.exit_code = pclose(fp);
    return cr;
#endif
}

bool service_restart(std::string& msg) {
    // Try daemon-reload (non-fatal), then restart service.
    (void)run_cmd("systemctl daemon-reload");
    auto r = run_cmd(std::string("systemctl restart ") + kService);
    if (r.exit_code != 0) {
        msg = "systemctl restart failed. Is the service name correct and do you have permission?";
        return false;
    }
    return true;
}

std::string read_journal_last_seconds(int seconds) {
    // Try without sudo first, then with sudo if empty (permission).
    std::ostringstream oss;
    oss << "journalctl -u " << kService
        << " --since \"-" << seconds << "s\" -o short-iso";
    auto r = run_cmd(oss.str());
    if (!r.out.empty()) return r.out;

    auto r2 = run_cmd(std::string("sudo ") + oss.str());
    return r2.out; // may be empty if sudo not permitted
}

bool contains_line(const std::string& hay, const char* needle) {
    return hay.find(needle) != std::string::npos;
}

bool contains_regex(const std::string& hay, const std::regex& re) {
    return std::regex_search(hay, re);
}

void spawn_short_process() {
#ifndef _WIN32
    // Fire-and-forget; we only need an EXEC event to hit netlink.
    // Use sh to background it and avoid blocking.
    (void)run_cmd("bash -lc 'sleep 1 & disown'");
#endif
}

} // namespace

// ---------------------------------------------------------------------
// TEST 1: Classifier init path (module load + dlopen + cc_init + netlink)
// ---------------------------------------------------------------------
MT_TEST(Classifier, InitPath, "integration") {
    using namespace std::chrono;

    // 1) Restart service for a clean window
    {
        std::string msg;
        bool ok = service_restart(msg);
        MT_REQUIRE(ctx, ok);
        (void)msg;
    }

    // 2) Wait a little to let URM print init logs
    std::this_thread::sleep_for(std::chrono::milliseconds(kSettleMs));

    // 3) Read recent logs
    const std::string logs = read_journal_last_seconds(kSinceSeconds);
    MT_REQUIRE(ctx, !logs.empty());

    // 4) Assertions on init path
    MT_REQUIRE(ctx, contains_line(logs, kInitTag1));   // registration path reached
    MT_REQUIRE(ctx, contains_line(logs, kInitTag2));   // dlopen+dlsym ok, entering cc_init()
    MT_REQUIRE(ctx, contains_line(logs, kInitTag3));   // ContextualClassifier::Init()
    MT_REQUIRE(ctx, contains_line(logs, kListenTag));  // Netlink set_listen(true)
}

// ---------------------------------------------------------------------
// TEST 2: Event delivery and worker classification
//         Launch `sleep 1` and assert we see a classification start log.
// ---------------------------------------------------------------------
MT_TEST(Classifier, EventDelivery, "integration") {
    // 1) Trigger an EXEC by creating a short-lived process
    spawn_short_process();

    // 2) Give netlink + worker a moment to process
    std::this_thread::sleep_for(std::chrono::milliseconds(kEventWaitMs));

    // 3) Read logs in the recent window
    const std::string logs = read_journal_last_seconds(kSinceSeconds);
    MT_REQUIRE(ctx, !logs.empty());

    // 4) At least one classification-start line must appear
    MT_REQUIRE(ctx, contains_regex(logs, kClassifyStartRe));

    // NOTE:
    // We intentionally DO NOT assert on signals or cgroup success to keep the test
    // robust on your Ubuntu box (those configs are device-specific).
}

// ---------------------------------------------------------------------
// TEST 3 (Optional, non-fatal): Observe a cgroup move attempt
//         This is not required to pass; we make it a soft expectation.
// ---------------------------------------------------------------------
MT_TEST(Classifier, CgroupAttempt_Optional, "integration") {
    spawn_short_process();
    std::this_thread::sleep_for(std::chrono::milliseconds(kEventWaitMs));

    const std::string logs = read_journal_last_seconds(kSinceSeconds);
    MT_REQUIRE(ctx, !logs.empty());

    // Soft check: either we saw a cgroup write attempt or the signals weren’t configured.
    const bool saw_move =
        logs.find("moveProcessToCGroup: Writing to Node:") != std::string::npos;
    const bool saw_signals_missing =
        logs.find("Signal ID [0x00000000] not found") != std::string::npos;

    if (!saw_move && !saw_signals_missing) {
        // Don’t fail the suite; mark as informational skip.
        MT_SKIP(ctx, "No cgroup attempt observed in window (ok on dev boxes).");
    }
    // If we’re here, test passes as “we saw one of the expected dev-box outcomes”.
}

// FastTextTests.cpp
//
// XFAIL tests for Contextual Classifier's FastText integration.
// These will FAIL by design on your current Ubuntu box (missing model/config),
// and thus be reported as XFAIL by mini.hpp. Once the model is present and
// inference is enabled, they will become XPASS (unexpected pass).
//
// Why these checks:
//  - MLInference logs "fastText model loaded. Embedding dimension: <n>"
//    after loading /etc/classifier/fasttext_model_supervised.bin.  (see MLInference.cpp)
//  - On successful prediction, it logs:
//      "Prediction complete. PID: <pid>, Comm: <name>, Class: <label>, Probability: <p>"
//    (see MLInference.cpp).                                            [1](https://qualcomm-my.sharepoint.com/personal/mamtas_qti_qualcomm_com/Documents/Microsoft%20Copilot%20Chat%20Files/MLInference.cpp)
//
// Model path the runtime uses (ContextualClassifier):
//   /etc/classifier/fasttext_model_supervised.bin                       [1](https://qualcomm-my.sharepoint.com/personal/mamtas_qti_qualcomm_com/Documents/Microsoft%20Copilot%20Chat%20Files/MLInference.cpp)

#include "mini.hpp"
#include <string>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <regex>
#include <thread>
#include <chrono>

namespace {

constexpr const char* kService = "urm";

// MLInference.cpp log strings (case-sensitive fragments we search for):
//  - "fastText model loaded. Embedding dimension:"        (model ready)
//  - "Prediction complete. PID:"                          (inference ran)  [1](https://qualcomm-my.sharepoint.com/personal/mamtas_qti_qualcomm_com/Documents/Microsoft%20Copilot%20Chat%20Files/MLInference.cpp)
constexpr const char* kModelLoadedTag = "fastText model loaded. Embedding dimension:";
constexpr const char* kPredictDoneTag = "Prediction complete. PID:";

// Read last N seconds from journal for the URM unit.
std::string read_journal_last_seconds(int seconds) {
    std::string cmd = "journalctl -u " + std::string(kService) +
                      " --since \"-" + std::to_string(seconds) + "s\" -o short-iso 2>/dev/null";
    auto* fp = popen(cmd.c_str(), "r");
    std::string out;
    if (fp) {
        std::array<char, 4096> buf{};
        while (size_t n = fread(buf.data(), 1, buf.size(), fp)) out.append(buf.data(), n);
        pclose(fp);
    }
    if (!out.empty()) return out;

    // Fallback with sudo (some hosts require elevated read)
    cmd = "sudo " + cmd;
    fp = popen(cmd.c_str(), "r");
    if (fp) {
        std::array<char, 4096> buf{};
        while (size_t n = fread(buf.data(), 1, buf.size(), fp)) out.append(buf.data(), n);
        pclose(fp);
    }
    return out;
}

bool service_restart() {
    // Best-effort daemon-reload (ignore rc)
    (void)system("systemctl daemon-reload >/dev/null 2>&1");
    int rc = system(("systemctl restart " + std::string(kService) + " >/dev/null 2>&1").c_str());
    return (rc == 0);
}

void spawn_short_process() {
    // Generate an EXEC event; prediction may or may not run depending on model.
    (void)system("bash -lc 'sleep 1 & disown' >/dev/null 2>&1");
}

} // namespace

// ------------------------------------------------------------------
// XFAIL: Expect model NOT to be loaded on dev box without the .bin.
// If the model is present later, this test will become XPASS.
// ------------------------------------------------------------------
MT_TEST_XFAIL(FastText, ModelLoads_XFail, "integration", "Model not installed on dev box") {
    // Restart to get a clean init window
    MT_REQUIRE(ctx, service_restart());
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    const std::string logs = read_journal_last_seconds(180);
    MT_REQUIRE(ctx, !logs.empty());

    // REQUIRE the "model loaded" line. If missing => failure => XFAIL (expected now).
    MT_REQUIRE(ctx, logs.find(kModelLoadedTag) != std::string::npos);
}

// ------------------------------------------------------------------
// XFAIL: Expect prediction NOT to happen on dev box (no model).
// If prediction logs appear later, this becomes XPASS.
// ------------------------------------------------------------------
MT_TEST_XFAIL(FastText, Predict_XFail, "integration", "Prediction disabled (no FastText model)") {
    // Don't restart here; assume URM is running. Trigger an event.
    spawn_short_process();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    const std::string logs = read_journal_last_seconds(120);
    MT_REQUIRE(ctx, !logs.empty());

    // REQUIRE at least one prediction completion log (case-sensitive fragment).
    MT_REQUIRE(ctx, logs.find(kPredictDoneTag) != std::string::npos);
}
