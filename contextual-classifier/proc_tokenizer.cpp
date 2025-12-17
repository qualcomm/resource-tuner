// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <parser.h>
#include <dirent.h>
#include <fstream>
#include <syslog.h> // Include for syslog

static std::string removeDatesAndTimesFromToken(const std::string& input) {
    std::string out = input;

    // Match common numeric date formats: dd/mm/yyyy, yyyy-mm-dd, etc.
    static const std::regex date_numeric(
        R"((\b\d{1,2}[-/.]\d{1,2}[-/.]\d{2,4}\b)|(\b\d{4}[-/.]\d{1,2}[-/.]\d{1,2}\b))",
        std::regex::ECMAScript | std::regex::icase);

    // Match month name formats: November 26, 2025 or 26 Nov 2025
    static const std::regex date_month_name(
        R"(\b(?:(?:jan(?:uary)?|feb(?:ruary)?|mar(?:ch)?|apr(?:il)?|may|jun(?:e)?|jul(?:y)?|aug(?:ust)?|sep(?:t|tember)?|oct(?:ober)?|nov(?:ember)?|dec(?:ember)?))\s+\d{1,2}(?:,\s*)?\s+\d{2,4}\b|\b\d{1,2}\s+(?:jan(?:uary)?|feb(?:ruary)?|mar(?:ch)?|apr(?:il)?|may|jun(?:e)?|jul(?:y)?|aug(?:ust)?|sep(?:t|tember)?|oct(?:ober)?|nov(?:ember)?|dec(?:ember)?)(?:,\s*)?\s+\d{2,4}\b)",
        std::regex::ECMAScript | std::regex::icase);

    // Match time formats: 13:45, 07:30:12, 7:30 PM
    static const std::regex time_hm(
        R"(\b\d{1,2}:\d{2}(:\d{2})?\s*(AM|PM)?\b)",
        std::regex::ECMAScript | std::regex::icase);

    // Remove matches
    out = std::regex_replace(out, date_numeric, "");
    out = std::regex_replace(out, date_month_name, "");
    out = std::regex_replace(out, time_hm, "");

    // Clean up extra spaces
    out = std::regex_replace(out, std::regex(R"(\s{2,})"), " ");

    return out;
}

bool isAllSpecialChars(const std::string& token) {
    if (token.empty()) return false; // let empty be handled elsewhere
    for (size_t i = 0; i < token.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(token[i]);
        if (std::isalnum(ch)) {
            return false; // has at least one alnum, so not "all special"
        }
    }
    return true; // no alnum found
}

// Helper: remove all punctuation from a token
static std::string removePunctuation(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(s[i]);
        if (!std::ispunct(ch)) {
            out.push_back(static_cast<char>(ch));
        }
    }
    return out;
}

// Helper: is the token a single character (after cleaning)?
static bool isSingleCharToken(const std::string& s) {
    return s.size() == 1;
}

bool hasDigit(const std::string& str) {
    std::regex digitRegex("[0-9]");
    return std::regex_search(str, digitRegex);
}

bool isDigitsOnly(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

// Trim leading and trailing whitespace
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::string normalizeLibraryName(const std::string& s) {
    std::string result = trim(s);
    if (result.empty()) return result;

    // If ".so" exists, keep everything before ".so"
    size_t pos = result.find(".so");
    if (pos != std::string::npos) {
        result = result.substr(0, pos);
    }

    // Remove trailing digits
    while (!result.empty() && std::isdigit(static_cast<unsigned char>(result.back()))) {
        result.pop_back();
    }

    // Remove trailing separators (-, _, .)
    while (!result.empty() && (result.back() == '-' || result.back() == '_' || result.back() == '.')) {
        result.pop_back();
    }

    // Do NOT strip "lib" prefix now
    result = trim(result);

    // Drop meaningless tokens
    if (result == "so") return "";

    return result;
}

static std::string escapeForCharClass(const std::string& d) {
    std::string out;
    out.reserve(d.size() * 2);
    for (char c : d) {
        switch (c) {
            case '\\':
            case ']':
            case '^':
            case '-':
                out.push_back('\\');
                [[fallthrough]];
            default:
                out.push_back(c);
        }
    }
    return out;
}

/* Function to split a string using multiple delimiters
   e.g., "[,;| ]" for delimiters , ; | and space
*/

std::vector<std::string> splitString(const std::string& input, const std::string& delimiters) {
    // Create a regex pattern for the delimiters
    std::string regexPattern = "[" + escapeForCharClass(delimiters) + "]"; 
    std::regex re(regexPattern);

    // Use regex_token_iterator to split the string
    std::sregex_token_iterator it(input.begin(), input.end(), re, -1);
    std::sregex_token_iterator end;

    // Collect the results into a vector
    std::vector<std::string> result;
    for (; it != end; ++it) {
        if (!it->str().empty()) { // Avoid empty tokens
            result.push_back(it->str());
        }
    }
    return result;
}

std::unordered_map<std::string, std::unordered_set<std::string>> loadIgnoreMap(const std::string& filename, const std::vector<std::string>& labels) {
    std::ifstream file(filename);
    std::string line;
    std::unordered_map<std::string, std::unordered_set<std::string>> ignoreMap;

    if (!file.is_open()) {
        syslog(LOG_ERR, "Error opening file: %s", filename.c_str());
        return ignoreMap;
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, values;

        if (std::getline(iss, key, ':') && std::getline(iss, values)) {
            // Only process keys in the labels list
            if (std::find(labels.begin(), labels.end(), key) != labels.end()) {
                std::istringstream valStream(values);
                std::string val;
                while (std::getline(valStream, val, ',')) {
                    // Trim whitespace from val
                    val.erase(0, val.find_first_not_of(" \t\n\r"));
                    val.erase(val.find_last_not_of(" \t\n\r") + 1);
                    if (!val.empty()) {
                        ignoreMap[key].emplace(val);
                    }
                }
            }
        }
    }

    return ignoreMap;
}

//filter process name and log
std::vector<std::string> extractProcessNameAndMessage(const std::vector<std::string>& journalLines) {
    std::vector<std::string> filtered;
    std::regex pattern(R"(.*? (\S+)\[(\d+)\]: (.*))");

    for (const auto& line : journalLines) {
        std::smatch match;
        if (std::regex_search(line, match, pattern) && match.size() == 4) {
            std::string processName = match[1];
            std::string message = match[3];
            filtered.push_back(processName + ": " + message);
        }
    }

    return filtered;
}

// Filter out strings present in the ignore set
std::vector<std::string> filterStrings(const std::vector<std::string>& input, const std::unordered_set<std::string>& ignoreSet) {
    std::vector<std::string> result;
    std::copy_if(input.begin(), input.end(), std::back_inserter(result),
                 [&ignoreSet](const std::string& s) {
                     return ignoreSet.find(s) == ignoreSet.end();
                 });
    return result;
}

std::vector<std::string> parse_proc_attr_current(const uint32_t pid, const std::string& delimiters) {
    std::vector<std::string> context_parts;
    std::string path = "/proc/" + std::to_string(pid) + "/attr/current";
    std::ifstream infile(path);
    if (!infile.is_open()) {
        syslog(LOG_ERR, "Failed to open %s", path.c_str());
        return context_parts;
    }

    std::string line;
    if (std::getline(infile, line)) {
        // Remove anything in parentheses like "(enforce)"
        line = std::regex_replace(line, std::regex(R"(\s*\(enforce\))"), "");

        // Split using provided delimiters
        context_parts = splitString(line, delimiters);
    }

    infile.close();
    return context_parts;
}


std::vector<std::string> parse_proc_cgroup(pid_t pid, const std::string& delimiters) {
    std::vector<std::string> tokens;
    std::string path = "/proc/" + std::to_string(pid) + "/cgroup";
    std::ifstream infile(path);
    if (!infile.is_open()) {
        syslog(LOG_ERR, "Failed to open %s", path.c_str());
        return tokens;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::vector<std::string> lineTokens = splitString(line, delimiters);
        tokens.insert(tokens.end(), lineTokens.begin(), lineTokens.end());
    }
    infile.close();
    return tokens;
}

std::vector<std::string> parse_proc_cmdline(pid_t pid, const std::string& delimiters) {
    std::vector<std::string> tokens;
    std::string path = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream infile(path, std::ios::binary);
    if (!infile.is_open()) {
        syslog(LOG_ERR, "Failed to open %s", path.c_str());
        return tokens;
    }

    std::string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    infile.close();

    size_t start = 0;
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\0') {
            if (i > start) {
                std::string arg = content.substr(start, i - start);
                for (const auto& raw : splitString(arg, delimiters)) {
                    std::string cleaned;
                    for (char c : raw) {
                        if (delimiters.find(c) == std::string::npos) {
                            cleaned += c;
                        }
                    }
                    cleaned = trim(cleaned);
                    if (!cleaned.empty() && !isDigitsOnly(cleaned) && cleaned.size() > 1) {
                        tokens.push_back(cleaned);
                    }
                }
            }
            start = i + 1;
        }
    }

    return tokens;
}

std::vector<std::string> parse_proc_comm(pid_t pid, const std::string& delimiters) {
    std::vector<std::string> tokens;
    std::string path = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream infile(path);
    if (!infile.is_open()) {
        syslog(LOG_ERR, "Failed to open %s", path.c_str());
        return tokens;
    }

    std::string comm;
    std::getline(infile, comm);
    infile.close();

    
    // Split and filter: remove empty or single-character tokens
    for (const auto& t : splitString(comm, delimiters)) {
        std::string cleaned = trim(t);
        if (!cleaned.empty() && cleaned.size() > 1) {
            tokens.push_back(cleaned);
        }
    }

    return tokens;
}

std::vector<std::string> parse_proc_map_files(pid_t pid, const std::string& delimiters) {
    std::vector<std::string> results;
    std::string dir_path = "/proc/" + std::to_string(pid) + "/map_files";
    DIR* dir = opendir(dir_path.c_str());
    if (!dir) {
        syslog(LOG_ERR, "Failed to open %s", dir_path.c_str());
        return results;
    }

    struct dirent* entry;
    char link_target[PATH_MAX];
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.') continue;

        std::string link_path = dir_path + "/" + entry->d_name;
        ssize_t len = readlink(link_path.c_str(), link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            std::string target_str(link_target);

            for (const auto& tok : splitString(target_str, delimiters)) {
                 
                std::string simplified = normalizeLibraryName(tok);
 
                // Skip empty or single-character tokens
                if (simplified.empty() || simplified.size() <= 1) continue;
                 
                // Skip if token is fully numeric (using isDigitsOnly)
                if (isDigitsOnly(simplified)) continue;
                
                // Deduplicate
                if (std::find(results.begin(), results.end(), simplified) == results.end()) {
                    results.push_back(simplified);
                }


            }
        }
    }

    closedir(dir);
    return results;
}

 // New function to parse /proc/<pid>/fd entries and extract relevant tokens
std::vector<std::string> parse_proc_fd(pid_t pid, const std::string& delimiters) {
    std::vector<std::string> results;
    std::string dir_path = "/proc/" + std::to_string(pid) + "/fd";
    DIR* dir = opendir(dir_path.c_str());
    if (!dir) {
        syslog(LOG_ERR, "Unable to open fd directory %s", dir_path.c_str());
        return results;
    }

    struct dirent* entry;
    char link_target[PATH_MAX];
    while ((entry = readdir(dir)) != nullptr) {
        // Skip . and ..
        if (entry->d_name[0] == '.') continue;

        std::string link_path = dir_path + "/" + entry->d_name;
        ssize_t len = readlink(link_path.c_str(), link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            std::string target_str(link_target);
            // Split the target string using provided delimiters
            syslog(LOG_DEBUG, "Parsing fd: %s", target_str.c_str());
            std::vector<std::string> tokens = splitString(target_str, delimiters);

            for (const auto& tok : tokens) {
                if (tok.empty()) continue;
                std::string cleaned = removeDatesAndTimesFromToken(tok);
                if (cleaned.empty()) continue;
                // Exclude pure numeric tokens (e.g., file descriptor numbers)
                bool isNumber = std::all_of(cleaned.begin(), cleaned.end(), ::isdigit);
                if (isNumber) continue;
                // Add unique tokens to results
                if (std::find(results.begin(), results.end(), cleaned) == results.end()) {
                    results.push_back(cleaned);
                }
            }
        }
    }
    closedir(dir);
    return results;
}

std::vector<std::string> parse_proc_environ(pid_t pid, const std::string& delimiters) {
    std::vector<std::string> out;
    std::string path = "/proc/" + std::to_string(pid) + "/environ";

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        syslog(LOG_ERR, "Failed to open: %s", path.c_str());
        return out;
    }

    std::string entry;
    while (std::getline(in, entry, '\0')) { // entries are null-separated
        if (entry.empty()) continue;

        std::vector<std::string> tokens = splitString(entry, delimiters);
        for (auto& token : tokens) {
            // Remove delimiter characters from the token manually
            for (size_t i = 0; i < token.size(); ) {
                if (delimiters.find(token[i]) != std::string::npos) {
                    token.erase(i, 1); // remove the delimiter character
                } else {
                    ++i;
                }
            }

            if (isAllSpecialChars(token)) continue; 
            // Filter out tokens that contain digits
            if (!token.empty() && !hasDigit(token)) {
                out.push_back(token);
            }

        }
    }

    return out;
}

std::vector<std::string> parse_proc_exe(pid_t pid, const std::string& delimiters) {
    std::vector<std::string> out;
    std::string path = "/proc/" + std::to_string(pid) + "/exe";
    char buf[PATH_MAX];
    ssize_t len = readlink(path.c_str(), buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        std::string exePath(buf);
        // Use your custom splitString function
        for (const auto& part : splitString(exePath, delimiters)) {
            if (!isDigitsOnly(part)) {
                out.push_back(part);
            }
        }
    } else {
        syslog(LOG_ERR, "Failed to readlink %s for PID %d", path.c_str(), pid);
    }

    return out;
}

std::vector<std::string> readJournalForPid(pid_t pid, uint32_t numLines) {
    std::vector<std::string> lines;
    
     
    std::string comm;
    std::ifstream commFile("/proc/" + std::to_string(pid) + "/comm");

    if (commFile.is_open()) {
        std::getline(commFile, comm);
        commFile.close();
    } else {
        syslog(LOG_ERR, "Failed to open /proc/%d/comm", pid);
        return lines;
    }

    std::string cmd = "journalctl --no-pager -n " + std::to_string(numLines) +
                      " _COMM=" + comm;


    //std::string cmd = "journalctl --no-pager -n 20 _PID=" + std::to_string(pid);

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        syslog(LOG_ERR, "popen failed: %m");
        return lines; // empty
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        lines.emplace_back(buffer);
    }

    int rc = pclose(pipe);
    if (rc == -1) {
        syslog(LOG_ERR, "pclose failed: %m");
    }

    return lines;
}

std::vector<std::string> parse_proc_log(const std::string& input, const std::string& delimiters) {
    std::vector<std::string> tokens;
    std::string token;
    std::unordered_set<char> delimSet(delimiters.begin(), delimiters.end());

    // Remove bracketed tags like [info], [warn], [error], even if they span lines
    std::regex bracketedTag(R"(\[\s*(info|warn|error|debug|trace)?\s*\]?)", std::regex_constants::icase);
    std::string cleaned_input = std::regex_replace(input, bracketedTag, "");

    // Normalize newlines and whitespace
    cleaned_input.erase(std::remove(cleaned_input.begin(), cleaned_input.end(), '\n'), cleaned_input.end());

    for (char ch : cleaned_input) {
        if (delimSet.count(ch)) {
            if (!token.empty()) {
               token = removePunctuation(token);
               if (!token.empty() && !isSingleCharToken(token) && !isDigitsOnly(token)) {
                      tokens.push_back(token);
               }
            }
            token.clear();
        } else {
            token += ch;
        }
    }

    if (!token.empty()) {
        token = removePunctuation(token);
        if (!token.empty() && !isSingleCharToken(token) && !isDigitsOnly(token)) {
            tokens.push_back(token);
        }
    }

    return tokens;
}
