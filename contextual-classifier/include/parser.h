#include <sstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <regex>
#include <unistd.h>
#include <cstdint>
#include <cctype>

#define LOG_LINES 20

//lables for Ignore strings.
const std::vector<std::string> entries = {"attr",
                                         "cgroup",
                                         "cmdline",
                                         "comm",
                                         "environ",
                                         "exe",
                                         "logs",
                                         "fds",
                                         "map_files"
                                         };

std::unordered_map<std::string, std::unordered_set<std::string>> loadIgnoreMap(const std::string& filename, const std::vector<std::string>& labels = entries);
std::vector<std::string> filterStrings(const std::vector<std::string>& input, const std::unordered_set<std::string>& ignoreSet);
std::vector<std::string> parse_proc_attr_current(const uint32_t pid, const std::string& delimiters);
std::vector<std::string> parse_proc_cgroup(pid_t pid, const std::string& delimiters);
std::vector<std::string> parse_proc_cmdline(pid_t pid, const std::string& delimiters);
std::vector<std::string> parse_proc_comm(pid_t pid, const std::string& delimiters);
std::vector<std::string> parse_proc_map_files(pid_t pid, const std::string& delimiters );
std::vector<std::string> parse_proc_fd(pid_t pid, const std::string& delimiters );
std::vector<std::string> parse_proc_environ(pid_t pid, const std::string& delimiters);
std::vector<std::string> parse_proc_exe(pid_t pid, const std::string& delimiters);
std::vector<std::string> extractProcessNameAndMessage(const std::vector<std::string>& journalLines);
std::vector<std::string> readJournalForPid(pid_t pid, uint32_t numLines = LOG_LINES);
std::vector<std::string> parse_proc_log(const std::string& input, const std::string& delimiters);
std::string trim(const std::string& s);
int collect_and_store_data(pid_t pid, const char* config_file);
