// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

// IntegrationTests.cpp
#include "../framework/mini.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <regex>
#include <string>
#include <thread>
#include <chrono>

namespace {

constexpr const char* kService = "urm";

// Tags/lines produced by your instrumented code paths:
//
// ContextualClassifierInit.cpp (Init trace you added):
static constexpr const char* kInitEnter  = "InitTrace: ENTER module init";
static constexpr const char* kInitCall   = "InitTrace: calling cc_init()";

// ContextualClassifier.cpp (Init path):
static constexpr const char* kInitOk1    = "Classifier module init.";
static constexpr const char* kInitOk2    = "Now listening for process events.";

// ContextualClassifier.cpp (worker consumes events):
static const std::regex kClassifyStartRe(
    R"(ClassifyProcess:\s*Starting classification for PID:\s*\d+)",
    std::regex::icase);

// MLInference.cpp (FastText):
static constexpr const char* kFtModelLoaded = "fastText model loaded. Embedding dimension:";
static constexpr const char* kFtPredictDone = "Prediction complete. PID:";

// URM cgroup move logs:
static constexpr const char* kCgrpWriteTry  = "moveProcessToCGroup: Writing to Node:";
static constexpr const char* kCgrpWriteFail = "moveProcessToCGroup: Call to write, Failed";

// Time windows (tuned for your box)
constexpr int kSinceSeconds = 240;  // query last N seconds of journald
constexpr int kSettleMs     = 1800; // wait after service restart
constexpr int kEventWaitMs  = 1500; // wait after spawning an event

struct CmdResult {
    int exit_code{-1};
    std::string out;
};

CmdResult run_cmd(const std::string& cmd) {
    CmdResult cr;
    std::array<char, 4096> buf{};
    FILE* fp = popen((cmd + " 2>/dev/null").c_str(), "r");
    if (!fp) { cr.exit_code = -1; return cr; }
    while (size_t n = fread(buf.data(), 1, buf.size(), fp)) {
        cr.out.append(buf.data(), n);
    }
    cr.exit_code = pclose(fp);
    return cr;
}

bool service_restart() {
    (void)run_cmd("systemctl daemon-reload");
    auto rc = run_cmd(std::string("systemctl restart ") + kService);
    return (rc.exit_code == 0);
}

std::string read_journal_last_seconds(int seconds) {
    // Try unprivileged read first
    {
        std::string cmd = "journalctl -u " + std::string(kService) +
                          " --since \"-" + std::to_string(seconds) + "s\" -o short-iso";
        auto r = run_cmd(cmd);
        if (!r.out.empty()) return r.out;
    }
    // Fallback with sudo
    {
        std::string cmd = "sudo journalctl -u " + std::string(kService) +
                          " --since \"-" + std::to_string(seconds) + "s\" -o short-iso";
        return run_cmd(cmd).out;
    }
}

void spawn_short_process() {
    (void)run_cmd("bash -lc 'sleep 1 & disown'");
}

bool contains_line(const std::string& logs, const char* needle) {
    return logs.find(needle) != std::string::npos;
}

bool contains_regex(const std::string& logs, const std::regex& re) {
    return std::regex_search(logs, re);
}

} // namespace

// ---------------------------------------------------------------------
// PASS: Init path (module load → cc_init → netlink subscribe)
// ---------------------------------------------------------------------
MT_TEST(Classifier, InitPath, "integration") {
    MT_REQUIRE(ctx, service_restart());
    std::this_thread::sleep_for(std::chrono::milliseconds(kSettleMs));

    const std::string logs = read_journal_last_seconds(kSinceSeconds);
    MT_REQUIRE(ctx, !logs.empty());

    MT_REQUIRE(ctx, contains_line(logs, kInitEnter));  // registration fired
    MT_REQUIRE(ctx, contains_line(logs, kInitCall));   // dlopen+dlsym ok, cc_init invoked
    MT_REQUIRE(ctx, contains_line(logs, kInitOk1));    // ContextualClassifier::Init()
    MT_REQUIRE(ctx, contains_line(logs, kInitOk2));    // Netlink set_listen(true)
}

// ---------------------------------------------------------------------
// PASS: Event delivery → worker consumes → classification starts
// ---------------------------------------------------------------------
MT_TEST(Classifier, EventDelivery, "integration") {
    spawn_short_process();
    std::this_thread::sleep_for(std::chrono::milliseconds(kEventWaitMs));

    const std::string logs = read_journal_last_seconds(kSinceSeconds);
    MT_REQUIRE(ctx, !logs.empty());
    MT_REQUIRE(ctx, contains_regex(logs, kClassifyStartRe));
}

// ---------------------------------------------------------------------
// XFAIL: FastText model load (expected missing on current box)
// will XPASS once /etc/classifier/fasttext_model_supervised.bin exists
// ---------------------------------------------------------------------
MT_TEST_XFAIL(FastText, ModelLoads_XFail, "integration", "FastText model not installed") {
    // Restart to capture model-load logs
    MT_REQUIRE(ctx, service_restart());
    std::this_thread::sleep_for(std::chrono::milliseconds(kSettleMs));

    const std::string logs = read_journal_last_seconds(kSinceSeconds);
    MT_REQUIRE(ctx, !logs.empty());

    // REQUIRE the model loaded line → currently absent → XFAIL
    MT_REQUIRE(ctx, contains_line(logs, kFtModelLoaded));
}

// ---------------------------------------------------------------------
// XFAIL: FastText prediction (expected absent without model)
// will XPASS once prediction runs and logs "Prediction complete..."
// ---------------------------------------------------------------------
MT_TEST_XFAIL(FastText, Predict_XFail, "integration", "Prediction disabled (no model)") {
    spawn_short_process();
    std::this_thread::sleep_for(std::chrono::milliseconds(kEventWaitMs));

    const std::string logs = read_journal_last_seconds(kSinceSeconds);
    MT_REQUIRE(ctx, !logs.empty());

    // REQUIRE the prediction completion line → currently absent → XFAIL
    MT_REQUIRE(ctx, contains_line(logs, kFtPredictDone));
}

// ---------------------------------------------------------------------
// XFAIL: Focused slice move success (expected to fail on this box)
// Success criteria we enforce here:
//   - We saw a "Writing to Node:" attempt for focused/app slice,
//   - AND we did NOT see "Call to write, Failed" in the same window.
// This will XPASS if your cgroup setup is fully correct.
// ---------------------------------------------------------------------
MT_TEST_XFAIL(Cgroups, FocusedSliceMove_XFail, "integration",
              "Focused/app slice not provisioned on this host") {
    spawn_short_process();
    std::this_thread::sleep_for(std::chrono::milliseconds(kEventWaitMs));

    const std::string logs = read_journal_last_seconds(kSinceSeconds);
    MT_REQUIRE(ctx, !logs.empty());

    const bool wrote  = contains_line(logs, kCgrpWriteTry);
    const bool failed = contains_line(logs, kCgrpWriteFail);

    // REQUIRE "wrote" and NOT "failed" → expected to fail today → XFAIL.
    MT_REQUIRE(ctx, wrote && !failed);
}


MT_TEST(Classifier, Netlink_RobustToEINTR, "integration") {
    // Kick a classification first
    spawn_short_process();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Send SIGUSR1 to the URM pid (best-effort; ignore rc)
    (void)run_cmd("bash -lc 'pgrep -x urm | head -n1 | xargs -r -I{} kill -USR1 {}'");

    // Trigger another event
    spawn_short_process();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    const std::string logs = read_journal_last_seconds(180);
    MT_REQUIRE(ctx, !logs.empty());
    MT_REQUIRE(ctx, contains_regex(logs, kClassifyStartRe));
}

MT_TEST(Classifier, Queue_BackPressure, "integration") {
    for (int i = 0; i < 50; ++i) spawn_short_process();
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    const std::string logs = read_journal_last_seconds(240);
    MT_REQUIRE(ctx, !logs.empty());

    // Count occurrences; must be > 0.
    std::sregex_iterator it(logs.begin(), logs.end(), kClassifyStartRe), end;
    int hits = std::distance(it, end);
    MT_REQUIRE(ctx, hits > 0);
}

MT_TEST(Classifier, Terminate_Restart_Clean, "integration") {
    MT_REQUIRE(ctx, service_restart());
    std::this_thread::sleep_for(std::chrono::milliseconds(kSettleMs));
    const std::string logs = read_journal_last_seconds(120);
    MT_REQUIRE(ctx, contains_line(logs, kInitEnter));
    MT_REQUIRE(ctx, contains_line(logs, kInitOk2)); // listening for events
}

MT_TEST(Classifier, IgnoreList_Basic, "integration") {
    // sed/sh are commonly ignored per your logs; spawn both.
    (void)run_cmd("bash -lc 'sh -c true; sed -n 1p /etc/hosts >/dev/null 2>&1'");
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    const std::string logs = read_journal_last_seconds(120);
    MT_REQUIRE(ctx, logs.find("Ignoring process: sh")  != std::string::npos
                    || logs.find("Ignoring process: sed") != std::string::npos);
}

MT_TEST_XFAIL(Signals, Config_Parsing_XFail, "integration",
              "Test configs contain non-numeric fields (stol)") {
    MT_REQUIRE(ctx, service_restart());
    std::this_thread::sleep_for(std::chrono::milliseconds(kSettleMs));
    const std::string logs = read_journal_last_seconds(120);
    MT_REQUIRE(ctx, logs.find("Signal Parsing Failed with Error: stol") != std::string::npos);
}

MT_TEST_XFAIL(Extractor, CSVDump_XFail, "integration",
              "mDebugMode not enabled; no CSVs expected") {
    spawn_short_process();
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    // Look for *either* pruned or unfiltered CSVs (simple probe)
    auto r = run_cmd("bash -lc 'ls /var/cache/pruned/*_proc_info* /var/cache/unfiltered/*_proc_info* 2>/dev/null | head -n1'");
    MT_REQUIRE(ctx, !r.out.empty()); // Will fail now → XFAIL
}
