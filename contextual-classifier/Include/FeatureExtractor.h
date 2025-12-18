// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef FEATURE_EXTRACTOR_H
#define FEATURE_EXTRACTOR_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <sys/types.h>

class FeatureExtractor {
public:
    static int collect_and_store_data(pid_t pid, const std::unordered_map<std::string, std::unordered_set<std::string>>& ignoreMap, std::map<std::string, std::string>& output_data, bool dump_csv);

private:
    static std::vector<std::string> parse_proc_attr_current(const uint32_t pid, const std::string& delimiters);
    static std::vector<std::string> parse_proc_cgroup(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> parse_proc_cmdline(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> parse_proc_comm(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> parse_proc_map_files(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> parse_proc_fd(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> parse_proc_environ(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> parse_proc_exe(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> readJournalForPid(pid_t pid, uint32_t numLines);
    static std::vector<std::string> parse_proc_log(const std::string& input, const std::string& delimiters);
    static std::vector<std::string> extractProcessNameAndMessage(const std::vector<std::string>& journalLines);
    static bool isValidPidViaProc(pid_t pid);
};

#endif // FEATURE_EXTRACTOR_H
