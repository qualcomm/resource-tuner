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
    static int CollectAndStoreData(pid_t pid,
        const std::unordered_map<std::string, std::unordered_set<std::string>>& ignoreMap,
        std::map<std::string, std::string>& output_data, bool dump_csv);

private:
    static std::vector<std::string> ParseAttrCurrent(const uint32_t pid, const std::string& delimiters);
    static std::vector<std::string> ParseCgroup(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> ParseCmdline(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> ParseComm(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> ParseMapFiles(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> ParseFd(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> ParseEnviron(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> ParseExe(pid_t pid, const std::string& delimiters);
    static std::vector<std::string> ReadJournalForPid(pid_t pid, uint32_t numLines);
    static std::vector<std::string> ParseLog(const std::string& input, const std::string& delimiters);
    static std::vector<std::string> ExtractProcessNameAndMessage(const std::vector<std::string>& journalLines);
    static bool IsValidPidViaProc(pid_t pid);
};

#endif // FEATURE_EXTRACTOR_H
