#include <unistd.h>
#include <syslog.h>
#include <parser.h>
#include <proc_stats.h>  // For system stats
#include <fstream>
#include <sys/stat.h>

#define PRUNED_DIR "/var/cache/pruned"
#define UNFILTERED_DIR "/var/cache/unfiltered"

void removeDoubleQuotes(std::vector<std::string>& vec) {
    for (auto& str : vec) {
        str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
    }
}

std::vector<std::string> toLowercaseVector(const std::vector<std::string>& input) {
    std::vector<std::string> result = input;
    for (size_t i = 0; i < result.size(); ++i) {
        for (size_t j = 0; j < result[i].size(); ++j) {
            result[i][j] = std::tolower(result[i][j]);
        }
    }
    return result;
}

void removeDoubleDash(std::vector<std::string>& vec) {
    auto it = vec.begin();
    while (it != vec.end()) {
        if (*it == "--") {
            // Remove the element if it's exactly "--"
            it = vec.erase(it);
        } else {
            // Remove all occurrences of "--" from the string
            size_t pos;
            while ((pos = it->find("--")) != std::string::npos) {
                it->erase(pos, 2);
            }
            ++it;
        }
    }
}

inline std::string remove_double_hyphen(const std::string& in) {
    std::string out = in;
    size_t pos;
    while ((pos = out.find("--")) != std::string::npos) {
        out.erase(pos, 2);
    }
    return out;
}

// 1) Replace canonical UUIDs (hex with dashes) -> "<N>"
static const std::regex kUuidRe(
    R"(\b[0-9a-fA-F]{8}(?:-[0-9a-fA-F]{4}){3}-[0-9a-fA-F]{12}\b)"
);

// 2) Replace hex-like runs (no 0x needed) -> "<N>"
//    Tune the {4,} threshold as you wish (4 is a good balance).
static const std::regex kHexRunRe(
    R"(\b[0-9a-fA-F]{4,}\b)"
);

// 3) Replace standalone decimal numbers -> "<N>"
static const std::regex kDecRe(
    R"(\b[+-]?\d+\b)"
);

inline std::string replace_numbers_and_hex_with_N(const std::string& in) {
    // Apply in an order that avoids double replacing parts of a UUID
    std::string s = std::regex_replace(in, kUuidRe, "n");
    s = std::regex_replace(s, kHexRunRe, "n");
    s = std::regex_replace(s, kDecRe, "n");
    return s;
}

// Your in-place loop over vector<string>
inline void normalize_numbers_inplace(std::vector<std::string>& tokens) {
    for (auto& s : tokens) {
        s = replace_numbers_and_hex_with_N(s);
    }
}

bool isValidPidViaProc(pid_t pid) {
    std::string procPath = "/proc/" + std::to_string(pid);
    struct stat info;

    // Check if the directory exists
    return (stat(procPath.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
}

int collect_and_store_data(pid_t pid, const char* config_file) {
    if(!isValidPidViaProc(pid)) {
        syslog(LOG_ERR, "PID %d does not exist in /proc.", pid);
        return 1;
    }
    std::string configFile = config_file;

    auto ignoreMap = loadIgnoreMap(configFile);
    syslog(LOG_DEBUG, "Ignore strings:");
    for (const auto& pair : ignoreMap) {
        std::string values;
        for (const auto& val : pair.second) {
            values += val + " ";
        }
        syslog(LOG_DEBUG, "%s: %s", pair.first.c_str(), values.c_str());
    }

    /*stats collection starts here */
    ProcStats procStats;
    MemStats memStats;
    IOStats ioStats;
    SocketStats socketstats;
    NwStats netStats;
    GpuStats gpuStats = {0, 0, 0, 0};
    DispStats dispStats;
    SchedStats schedStats;

    std::vector<int> processInodes;
    std::vector<SocketStats> tcpSockets;
    std::vector<SocketStats> udpSockets;
    std::vector<SocketStats> matchedSockets;

    FetchProcStats(pid, procStats);
    FetchMemStats(pid, memStats);
    FetchIOStats(pid, ioStats);
    FetchNwStats(pid, netStats);
    FetchGpuStats(gpuStats);
    FetchDisplayStats(dispStats);
    read_schedstat(pid, schedStats);

    std::vector<std::string> systemDisplays;
    int total = getActiveDisplays(systemDisplays);

    getProcessSocketInodes(pid, processInodes);
    parseNetFile("/proc/net/tcp", tcpSockets);
    parseNetFile("/proc/net/udp", udpSockets);

    for (const auto& sock : tcpSockets) {
        if (std::find(processInodes.begin(), processInodes.end(), sock.inode) != processInodes.end()) {
            matchedSockets.push_back(sock);
        }
    }
    for (const auto& sock : udpSockets) {
        if (std::find(processInodes.begin(), processInodes.end(), sock.inode) != processInodes.end()) {
            matchedSockets.push_back(sock);
        }
    }

    /* Parse attr_current */
    /*
     QLI has below format: SELinux
       system_u:system_r:qcom_cam_server_t:s0
       <user>:<role>:<type>:<level>

     Ubuntu has below format: AppArmor
        snap.chromium.chromium (enforce)
        snap.chromium.chromium → This is the AppArmor profile name for the Chromium browser installed via Snap.
    */
    std::string delimiters = ".:";
    std::vector<std::string> context = parse_proc_attr_current(pid, delimiters);
    if(!context.empty()) {
        syslog(LOG_DEBUG, "attr_current:");
        for(const auto& c: context) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    std::vector<std::string> lowercontext = toLowercaseVector(context);
    auto filtered_context = filterStrings(lowercontext, ignoreMap["attr"]);
    if(!filtered_context.empty()) {
        syslog(LOG_DEBUG, "filtered attr_current:");
        for(const auto& c: filtered_context) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }

    /* Parse cgroup */
    /*
        hierarchy-ID:controller-list:cgroup-path
    */
    delimiters = ":\"/";
    std::vector<std::string> cgroup = parse_proc_cgroup(pid, delimiters);
    if(!cgroup.empty()) {
        syslog(LOG_DEBUG, "cgroup:");
        for(const auto& c: cgroup) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    std::vector<std::string> lowercgroup = toLowercaseVector(cgroup);
    auto filtered_cg = filterStrings(lowercgroup, ignoreMap["cgroup"]);
    if(!filtered_cg.empty()) {
        syslog(LOG_DEBUG, "filtered cg:");
        for(const auto& c: filtered_cg) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }

    normalize_numbers_inplace(filtered_cg);
    if(!filtered_cg.empty()) {
        syslog(LOG_DEBUG, "filtered cg:");
        for(const auto& c: filtered_cg) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    
    /* Parse cmdline */
    /*
        ENV variable = value
        --config-a=vaule

    */
    delimiters = ".=/!";
    std::vector<std::string> cmdline = parse_proc_cmdline(pid, delimiters);
    if(!cmdline.empty()) {
        syslog(LOG_DEBUG, "cmdline:");
        for(const auto& c: cmdline) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    std::vector<std::string> lowercmdline = toLowercaseVector(cmdline);
    //TODO: filter single digit numbers.
    auto filtered_cmd = filterStrings(lowercmdline, ignoreMap["cmdline"]);
    if(!filtered_cmd.empty()) {
        syslog(LOG_DEBUG, "filtered cmdline:");
        for(const auto& c: filtered_cmd) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    removeDoubleDash(filtered_cmd);
    if(!filtered_cmd.empty()) {
        syslog(LOG_DEBUG, "filtered cmdline:");
        for(const auto& c: filtered_cmd) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }

    /* Parse comm */
    delimiters = ".";
    std::vector<std::string> comm = parse_proc_comm(pid, delimiters);
    if(!comm.empty()) {
        syslog(LOG_DEBUG, "comm:");
        for(const auto& c: comm) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    std::vector<std::string> lowercomm = toLowercaseVector(comm);
    auto filtered_comm = filterStrings(lowercomm, ignoreMap["comm"]);
    if(!filtered_comm.empty()) {
        syslog(LOG_DEBUG, "filtered comm:");
        for(const auto& c: filtered_comm) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    normalize_numbers_inplace(filtered_comm);

    /* parse map_files */
    delimiters = "/()_:.";
    std::vector<std::string> maps = parse_proc_map_files(pid, delimiters);
    if(!maps.empty()) {
        syslog(LOG_DEBUG, "map_files:");
        for(const auto& c: maps) {
		    syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    std::vector<std::string> lowermaps = toLowercaseVector(maps);
    auto filtered_maps = filterStrings(lowermaps, ignoreMap["map_files"]);
    if(!filtered_maps.empty()) {
        syslog(LOG_DEBUG, "filtered map_files:");
        for(const auto& c: filtered_maps) {
			syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    normalize_numbers_inplace(filtered_maps);

    /* parse fd */
    delimiters = ":[]/()=";
    std::vector<std::string> fds = parse_proc_fd(pid, delimiters);
    if(!fds.empty()) {
        syslog(LOG_DEBUG, "fds:");
        for(const auto& c: fds) {
		    syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    std::vector<std::string> lowerfds = toLowercaseVector(fds);
    auto filtered_fds = filterStrings(lowerfds, ignoreMap["fds"]);
    if(!filtered_fds.empty()) {
        syslog(LOG_DEBUG, "filtered fds:");
        for(const auto& c: filtered_fds) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }

    // parse environ with delimiters
    delimiters = "=@;!-._/:, ";
    std::vector<std::string> environ = parse_proc_environ(pid, delimiters);
    if (!environ.empty()) {
        syslog(LOG_DEBUG, "environ:");
        for (const auto& c : environ) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    std::vector<std::string> lowerenviron = toLowercaseVector(environ);
    auto filtered_environ = filterStrings(lowerenviron, ignoreMap["environ"]);
    if (!filtered_environ.empty()) {
        syslog(LOG_DEBUG, "filtered environ:");
        for (const auto& c : filtered_environ) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    normalize_numbers_inplace(filtered_environ);
    if(!filtered_environ.empty()) {
        syslog(LOG_DEBUG, "filtered environ:");
        for(const auto& c: filtered_environ) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }

    delimiters = "/.";
    std::vector<std::string> exe = parse_proc_exe(pid, delimiters);
    if(!exe.empty()) {
        syslog(LOG_DEBUG, "exe");
        for(const auto& c: exe) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        } 
    }
    std::vector<std::string> lowerexe = toLowercaseVector(exe);
    auto filtered_exe = filterStrings(lowerexe, ignoreMap["exe"]);
    if(!filtered_exe.empty()) {
        syslog(LOG_DEBUG, "filtered exe:");
        for(const auto& c: filtered_exe) {
            syslog(LOG_DEBUG, "%s", c.c_str());
        }
    }
    normalize_numbers_inplace(filtered_exe);

    delimiters = "=!'&/.,:- ";
    /* Read log using journalctl */
    auto journalctl_logs = readJournalForPid(pid, LOG_LINES);
    if (journalctl_logs.empty()) {
       syslog(LOG_DEBUG, "No logs found for PID %d", pid);
    }

    // Ignore first 3 columns in journalctl logs
    auto extracted_Logs = extractProcessNameAndMessage(journalctl_logs);
    syslog(LOG_DEBUG, "Filtered log entries for PID %d:", pid);

    std::vector<std::string> logs;

    for (const auto& entry : extracted_Logs) {
         syslog(LOG_DEBUG, "%s", entry.c_str());

         // Tokenize the filtered log entry
          auto tokens = parse_proc_log(entry, delimiters);

          syslog(LOG_DEBUG, "logs");
          for (const auto& c : tokens) {
               syslog(LOG_DEBUG, "%s", c.c_str());
               logs.push_back(c); // Accumulate tokens into logs
           }
    }

    std::vector<std::string> lowerlogs = toLowercaseVector(logs);

    // Now logs contains tokens from all entries
    auto filtered_logs = filterStrings(lowerlogs, ignoreMap["logs"]);
    if (!filtered_logs.empty()) {
        syslog(LOG_DEBUG, "filtered logs:");
        for (const auto& c : filtered_logs) {
            syslog(LOG_DEBUG, "%s", c.c_str());
         }
   }    

   removeDoubleQuotes(filtered_logs);

    std::string prunedFolder = PRUNED_DIR;
    std::string unfilteredFolder = UNFILTERED_DIR;

    // Create pruned folder if it doesn't exist
    if (access(prunedFolder.c_str(), F_OK) != 0) {
       mkdir(prunedFolder.c_str(), 0755);
       syslog(LOG_INFO, "New folder created: %s", prunedFolder.c_str());
    }

    // Create unfiltered folder if it doesn't exist
    if (access(unfilteredFolder.c_str(), F_OK) != 0) {
       mkdir(unfilteredFolder.c_str(), 0755);
    }
    std::string processName = comm.empty() ? "unknown_process" : comm[0];
    for (size_t i = 0; i < processName.size(); ++i) {
       processName[i] = std::tolower(processName[i]);
    }

    std::string fileName = processName + "_" + std::to_string(pid) + "_proc_info.csv";
    syslog(LOG_DEBUG, "FileName: %s", fileName.c_str());

    // -------------------- UNFILTERED FILE --------------------
    std::string unfilteredFile = unfilteredFolder + "/" + fileName + "_unfiltered.csv";
    std::ofstream unfilteredCSV(unfilteredFile);
    if (!unfilteredCSV.is_open()) {
        syslog(LOG_ERR, "Failed to open unfiltered file: %s", unfilteredFile.c_str());
    } else {
        unfilteredCSV << "PID,attr,cgroup,cmdline,comm,maps,fds,environ,exe,logs,cpu_time,threads,rss,vms,mem_vmpeak,mem_vmlck,mem_hwm,mem_vm_rss,mem_vmsize,mem_vmdata,mem_vmstk,mem_vm_exe,mem_vmlib,mem_vmpte,mem_vmpmd,mem_vmswap,mem_thread,read_bytes,write_bytes,tcp_tx,tcp_rx,udp_tx,udp_rx,gpu_busy,gpu_mem_allocated,display_on,active_displays,runtime_ns,rq_wait_ns,timeslices\n";
        unfilteredCSV << pid;

        // attr
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        std::vector<std::string> lowerContext = toLowercaseVector(context);
        for (size_t i = 0; i < lowerContext.size(); ++i) {
            unfilteredCSV << lowerContext[i];
            if (i != lowerContext.size() - 1) unfilteredCSV << ",";
        }
        unfilteredCSV << '"';

        // cgroup
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        std::vector<std::string> lowercgroup = toLowercaseVector(cgroup);
        for (size_t i = 0; i < lowercgroup.size(); ++i) {
            unfilteredCSV << lowercgroup[i];
            if (i != lowercgroup.size() - 1) unfilteredCSV << ",";
        }
        unfilteredCSV << '"';

        // cmdline
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        std::vector<std::string> lowercmdline = toLowercaseVector(cmdline);
        for (size_t i = 0; i < lowercmdline.size(); ++i) {
            unfilteredCSV << lowercmdline[i];
            if (i != lowercmdline.size() - 1) unfilteredCSV << ",";
        }
        unfilteredCSV << '"';

        // comm
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        std::vector<std::string> lowercomm = toLowercaseVector(comm);
        for (size_t i = 0; i < lowercomm.size(); ++i) {
            unfilteredCSV << lowercomm[i];
            if (i != lowercomm.size() - 1) unfilteredCSV << ",";
        }
        unfilteredCSV << '"';

        // maps
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        std::vector<std::string> lowermaps = toLowercaseVector(maps);
        for (size_t i = 0; i < lowermaps.size(); ++i) {
            unfilteredCSV << lowermaps[i];
            if (i != lowermaps.size() - 1) unfilteredCSV << ",";
        }
        unfilteredCSV << '"';

        // fds
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        std::vector<std::string> lowerfds = toLowercaseVector(fds);
        for (size_t i = 0; i < lowerfds.size(); ++i) {
            unfilteredCSV << lowerfds[i];
            if (i != lowerfds.size() - 1) unfilteredCSV << ",";
        }
        unfilteredCSV << '"';

        // environ
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        std::vector<std::string> lowerenviron = toLowercaseVector(environ);
        for (size_t i = 0; i < lowerenviron.size(); ++i) {
             for (char ch : lowerenviron[i]) {
                 if (ch == '"') unfilteredCSV << "\"\""; // escape quotes
                 else unfilteredCSV << ch;
             }
            if (i != lowerenviron.size() - 1) unfilteredCSV << ",";
        }
        unfilteredCSV << '"';
        
        // exe
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        std::vector<std::string> lowerexe = toLowercaseVector(exe);
        for (size_t i = 0; i < lowerexe.size(); ++i) {
            unfilteredCSV << lowerexe[i];
            if (i != lowerexe.size() - 1) unfilteredCSV << ",";
        }
        unfilteredCSV << '"';
       
        // logs
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        std::vector<std::string> lowerlogs = toLowercaseVector(logs);
        for (size_t i = 0; i < lowerlogs.size(); ++i) {
            unfilteredCSV << lowerlogs[i];
            if (i != lowerlogs.size() - 1) unfilteredCSV << ",";
        }
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << procStats.cpu_time;
        unfilteredCSV << '"';
        
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << procStats.num_threads;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << procStats.memory_rss;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << procStats.memory_vms;
        unfilteredCSV << '"';
        
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_peak;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_lck;
        unfilteredCSV << '"';


        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_hwm;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_rss;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_size;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_data;
        unfilteredCSV << '"';


        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_stk;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_exe;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_lib;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_pte;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_pmd;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.vm_swap;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << memStats.threads;
        unfilteredCSV << '"';


        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << ioStats.read_bytes;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << ioStats.write_bytes;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << netStats.tcp_tx;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << netStats.tcp_rx;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << netStats.udp_tx;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << netStats.udp_rx;
        unfilteredCSV << '"';
       
        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << gpuStats.busy_percent;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << gpuStats.mem_allocated;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << dispStats.display_on;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << total;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << schedStats.runtime_ns;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << schedStats.rq_wait_ns;
        unfilteredCSV << '"';

        unfilteredCSV << ',';
        unfilteredCSV << '"';
        unfilteredCSV << schedStats.timeslices;
        unfilteredCSV << '"';

        unfilteredCSV << '\n';
        unfilteredCSV.close();
    }

    // -------------------- FILTERED FILE --------------------
    std::string filteredFile = prunedFolder + "/" + fileName + "_filtered.csv";
    std::ofstream filteredCSV(filteredFile);
    if (!filteredCSV.is_open()) {
        syslog(LOG_ERR, "Failed to open filtered file: %s", filteredFile.c_str());
    } else {
        filteredCSV << "PID,attr,cgroup,cmdline,comm,maps,fds,environ,exe,logs,cpu_time,threads,rss,vms,mem_vmpeak,mem_vmlck,mem_hwm,mem_vm_rss,mem_vmsize,mem_vmdata,mem_vmstk,mem_vm_exe,mem_vmlib,mem_vmpte,mem_vmpmd,mem_vmswap,mem_thread,read_bytes,write_bytes,tcp_tx,tcp_rx,udp_tx,udp_rx,gpu_busy,gpu_mem_allocated,display_on,active_displays,runtime_ns,rq_wait_ns,timeslices\n";
        filteredCSV << pid;

        // attr
        filteredCSV << ',';
        filteredCSV << '"';
        for (size_t i = 0; i < filtered_context.size(); ++i) {
            filteredCSV << filtered_context[i];
            if (i != filtered_context.size() - 1) filteredCSV << ",";
        }
        filteredCSV << '"';

        // cgroup
        filteredCSV << ',';
        filteredCSV << '"';
        for (size_t i = 0; i < filtered_cg.size(); ++i) {
            filteredCSV << filtered_cg[i];
            if (i != filtered_cg.size() - 1) filteredCSV << ",";
        }
        filteredCSV << '"';

        // cmdline
        filteredCSV << ',';
        filteredCSV << '"';
        for (size_t i = 0; i < filtered_cmd.size(); ++i) {
            filteredCSV << filtered_cmd[i];
            if (i != filtered_cmd.size() - 1) filteredCSV << ",";
        }
        filteredCSV << '"';

        // comm
        filteredCSV << ',';
        filteredCSV << '"';
        for (size_t i = 0; i < filtered_comm.size(); ++i) {
            filteredCSV << filtered_comm[i];
            if (i != filtered_comm.size() - 1) filteredCSV << ",";
        }
        filteredCSV << '"';

        // maps
        filteredCSV << ',';
        filteredCSV << '"';
        for (size_t i = 0; i < filtered_maps.size(); ++i) {
            filteredCSV << filtered_maps[i];
            if (i != filtered_maps.size() - 1) filteredCSV << ",";
        }
        filteredCSV << '"';

        // fds
        filteredCSV << ',';
        filteredCSV << '"';
        for (size_t i = 0; i < filtered_fds.size(); ++i) {
            filteredCSV << filtered_fds[i];
            if (i != filtered_fds.size() - 1) filteredCSV << ",";
        }
        filteredCSV << '"';

        // environ
        filteredCSV << ',';
        filteredCSV << '"';
        for (size_t i = 0; i < filtered_environ.size(); ++i) {
            for (char ch : filtered_environ[i]) {
                 if (ch == '"') filteredCSV << "\"\""; // escape quotes
                  else filteredCSV << ch;
             }
            if (i != filtered_environ.size() - 1) filteredCSV << ",";
        }
        filteredCSV << '"';

        // exe
        filteredCSV << ',';
        filteredCSV << '"';
        for (size_t i = 0; i < filtered_exe.size(); ++i) {
            filteredCSV << filtered_exe[i];
            if (i != filtered_exe.size() - 1) filteredCSV << ",";
        }
        filteredCSV << '"';

	    // logs
        filteredCSV << ',';
        filteredCSV << '"';
        for (size_t i = 0; i < filtered_logs.size(); ++i) {
            filteredCSV << filtered_logs[i];
            if (i != filtered_logs.size() - 1) filteredCSV << ",";
        }
        filteredCSV << '"';

       // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << procStats.cpu_time;
        filteredCSV << '"';
        
        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << procStats.num_threads;
        filteredCSV << '"';

        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << procStats.memory_rss;
        filteredCSV << '"';

        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << procStats.memory_vms;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_peak;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_lck;
        filteredCSV << '"';


        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_hwm;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_rss;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_size;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_data;
        filteredCSV << '"';


        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_stk;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_exe;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_lib;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_pte;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_pmd;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.vm_swap;
        filteredCSV << '"';

        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << memStats.threads;
        filteredCSV << '"';



        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << ioStats.read_bytes;
        filteredCSV << '"';

        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << ioStats.write_bytes;
        filteredCSV << '"';

       // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << netStats.tcp_tx;
        filteredCSV << '"';

       // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << netStats.tcp_rx;
        filteredCSV << '"';

      // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << netStats.udp_tx;
        filteredCSV << '"';

        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << netStats.udp_rx;
        filteredCSV << '"';
       
        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << gpuStats.busy_percent;
        filteredCSV << '"';

        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << gpuStats.mem_allocated;
        filteredCSV << '"';

        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << dispStats.display_on;
        filteredCSV << '"';

        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << dispStats.display_on;
        filteredCSV << '"';

       // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << schedStats.runtime_ns;
        filteredCSV << '"';

        // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << schedStats.rq_wait_ns;
        filteredCSV << '"';

       // Write system stats
        filteredCSV << ',';
        filteredCSV << '"';
        filteredCSV << schedStats.timeslices;
        filteredCSV << '"';

        filteredCSV << '\n';
        filteredCSV.close();
    }

    return 0;
}
