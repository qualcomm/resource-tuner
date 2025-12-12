#include <proc_stats.h>
#include <syslog.h>
#include <fstream> // Added for std::ifstream
#include <string> // Ensure string is included for std::string
#include <fcntl.h> // For open
#include <sys/ioctl.h> // For ioctl
#include <cstring> // For strerror
#include <errno.h> // For errno

std::string readFile(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void FetchProcStats(int pid, ProcStats &stats) {
    stats = ProcStats{};
    stats.pid = pid;
    std::string statContent = readFile("/proc/" + std::to_string(pid) + "/stat");
    if (!statContent.empty()) {
        std::istringstream iss(statContent);
        std::string token;
        int count = 0;
        while (iss >> token) {
            count++;
            switch(count) {
                case 7:
                    stats.tty_nr_exists = (std::stoi(token) != 0);
                    break;
                case 8:
                    stats.tpgid_exists = (std::stoi(token) > 0);
                    break;
                case 10:
                    stats.minflt = std::stol(token);
                    break;
                case 11:
                    stats.majflt = std::stol(token);
                    break;
                case 14:
                    stats.utime = std::stol(token);
		            syslog(LOG_DEBUG, "utime:%f", stats.utime);
                    break;
                case 15:
                    stats.stime = std::stol(token);
		            syslog(LOG_DEBUG, "stime:%f", stats.stime);
                    break;
                case 18:
                    stats.priority = std::stoi(token);
                    break;
                case 19:
                    stats.nice = std::stoi(token);
                    break;
                case 20:
                    stats.num_threads = std::stoi(token);
                    break;
                case 23:
                     stats.memory_vms = std::stoll(token);
                     break;
                case 24:
                     stats.memory_rss = std::stoll(token);
                     break;
                case 40:
                    stats.rt_priority = std::stoi(token);
                    break;
                case 41:
                    stats.policy = std::stoi(token);
                    break;
                case 42:
                    stats.delayacct_blkio_ticks = std::stod(token);
                    break;
                default:
                    break;
            } /*switch(count)*/
        } /*while (iss >> token)*/
        stats.cpu_time = (stats.utime + stats.stime) / sysconf(_SC_CLK_TCK); // seconds
    } /*if (!statContent.empty())*/
    // Foreground heuristic: check if fd/0 exists
    std::string fdPath = "/proc/" + std::to_string(pid) + "/fd/0";
    struct stat buffer;
    stats.fg = (stat(fdPath.c_str(), &buffer) == 0);
    return;
}

long extractValue(const std::string& line) {
    std::istringstream iss(line);
    std::string key;
    long value = 0;
    std::string unit;

    // Expected format: VmPeak: <number> kB
    iss >> key >> value >> unit;

    return value; // value is in kilobytes
}

void FetchMemStats(int pid, MemStats &memstats) {
    memset(&memstats, 0, sizeof(MemStats));
    std::string statusContent = readFile("/proc/" + std::to_string(pid) + "/status");
    std::istringstream statusStream(statusContent);
    std::string line;
    while (std::getline(statusStream, line)) {
        if (line.find("Uid:") == 0) {
            memstats.is_app = (std::stol(line.substr(6)) > 1000);
        }
        else if (line.find("VmPeak:") == 0) {
            memstats.vm_peak = extractValue(line);
            syslog(LOG_DEBUG, "VmPeak:%ld", memstats.vm_peak);
        }
        else if (line.find("VmLck:") == 0) {
            memstats.vm_lck = extractValue(line); // in kb
            syslog(LOG_DEBUG, "VmLck:%ld", memstats.vm_lck);
        }
        else if (line.find("VmHWM:") == 0) {
            memstats.vm_hwm = extractValue(line); // in kb
            syslog(LOG_DEBUG, "VmHWM:%ld", memstats.vm_hwm);
        }  
        else if (line.find("VmRSS:") == 0) {
            memstats.vm_rss = extractValue(line); // in kb
            syslog(LOG_DEBUG, "VmRSS:%ld", memstats.vm_rss);
        }
        else if (line.find("VmSize:") == 0) {
            memstats.vm_size = extractValue(line); 
            syslog(LOG_DEBUG, "VmSize:%ld", memstats.vm_size);
            
        }
        else if (line.find("VmData:") == 0) {
            memstats.vm_data = extractValue(line);
            syslog(LOG_DEBUG, "VmData:%ld", memstats.vm_data);
        }
        else if (line.find("VmStk:") == 0) {
            memstats.vm_stk = extractValue(line);
            syslog(LOG_DEBUG, "VmStk:%ld", memstats.vm_stk);
        }
        else if (line.find("VmExe:") == 0) {
            memstats.vm_exe = extractValue(line);
            syslog(LOG_DEBUG, "VmExe:%ld", memstats.vm_exe);
        }
        else if (line.find("VmLib:") == 0) {
            memstats.vm_lib = extractValue(line);
            syslog(LOG_DEBUG, "VmLib:%ld", memstats.vm_lib);
        }
        else if (line.find("VmPTE:") == 0) {
            memstats.vm_pte = extractValue(line);
            syslog(LOG_DEBUG, "VmPTE:%ld", memstats.vm_pte);
        }
        else if (line.find("VmPMD:") == 0) {
            memstats.vm_pmd = extractValue(line);
            syslog(LOG_DEBUG, "VmPMD:%ld", memstats.vm_pmd);
        }
        else if (line.find("VmSwap:") == 0) {
            memstats.vm_swap = extractValue(line);
            syslog(LOG_DEBUG, "VmSwap:%ld", memstats.vm_swap);
        }
        else if (line.find("Threads:") == 0) {
            memstats.threads = extractValue(line);
            syslog(LOG_DEBUG, "Threads:%ld", memstats.threads);
        }
    }
}

// Count open file descriptors by type
void countFDTypes(int pid, int &fileCount, int &socketCount, int &pipeCount, int &charDevCount) {
    fileCount = socketCount = pipeCount = charDevCount = 0;

    std::string fdPath = "/proc/" + std::to_string(pid) + "/fd";
    DIR *dir = opendir(fdPath.c_str());
    if (!dir) {
        syslog(LOG_ERR, "Failed to open %s: %m", fdPath.c_str());
        return;
    }

    struct dirent *entry;
    char linkTarget[256];
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_LNK) {
            std::string fullPath = fdPath + "/" + entry->d_name;
            ssize_t len = readlink(fullPath.c_str(), linkTarget, sizeof(linkTarget) - 1);
            if (len != -1) {
                linkTarget[len] = '\0';
                std::string target(linkTarget);

                if (target.find("socket:[") != std::string::npos) {
                    socketCount++;
                } else if (target.find("pipe:[") != std::string::npos) {
                    pipeCount++;
                } else {
                    struct stat st;
                    if (stat(fullPath.c_str(), &st) == 0) {
                        if (S_ISCHR(st.st_mode)) {
                            charDevCount++;
                        } else {
                            fileCount++;
                        }
                    }
                }
            }
        }
    }
    closedir(dir);
}

// Count anonymous memory maps from /proc/<pid>/maps
int countAnonMemoryMaps(int pid) {
    std::string mapsPath = "/proc/" + std::to_string(pid) + "/maps";
    std::ifstream file(mapsPath);
    if (!file.is_open()) return -1;

    int anonCount = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("[anon]") != std::string::npos || line.find("heap") != std::string::npos) {
            anonCount++;
        }
    }
    return anonCount;
}

void FetchIOStats(int pid, IOStats &iostats) {
    memset(&iostats, 0, sizeof(IOStats));
    std::string line;
    int fileCount, socketCount, pipeCount, charDevCount, anonMaps;

    // I/O stats from /proc/[pid]/io
    std::string ioContent = readFile("/proc/" + std::to_string(pid) + "/io");
    std::istringstream ioStream(ioContent);
    while (std::getline(ioStream, line)) {
        if (line.find("read_bytes:") == 0) {
            iostats.read_bytes = std::stoul(line.substr(12));
        }
        if (line.find("write_bytes:") == 0) {
            iostats.write_bytes = std::stoul(line.substr(13));
        }
    }

    countFDTypes(pid, fileCount, socketCount, pipeCount, charDevCount);
    anonMaps = countAnonMemoryMaps(pid);

    iostats.open_file_count = fileCount;
    iostats.sock_count = socketCount;
    iostats.pipe_count = pipeCount;
    iostats.chardev_count = charDevCount;
    iostats.anonmaps_count = anonMaps;
}

int parseNetFile(const std::string &path, std::vector<SocketStats> &sockets) {
    std::ifstream file(path);
    if (!file.is_open()) return -1;

    std::string line;
    bool headerSkipped = false;
    while (std::getline(file, line)) {
        if (!headerSkipped) { headerSkipped = true; continue; } // skip header
        std::istringstream iss(line);
        SocketStats s;
        std::string sl, local, remote, st, txrx;
        iss >> sl >> local >> remote >> st >> txrx;
        for (int i = 0; i < 6; i++) iss >> sl; // skip unused fields
        iss >> s.inode;

        s.local_addr = local;
        s.remote_addr = remote;
        s.state = st;

        // txrx format: tx:rx in hex
        size_t colonPos = txrx.find(':');
        s.tx_queue = std::stoul(txrx.substr(0, colonPos), nullptr, 16);
        s.rx_queue = std::stoul(txrx.substr(colonPos + 1), nullptr, 16);

        sockets.push_back(s);
    }
    return 0;
}

int getProcessSocketInodes(int pid, std::vector<int> &inodes) {
    std::string fdPath = "/proc/" + std::to_string(pid) + "/fd";
    DIR *dir = opendir(fdPath.c_str());
    if (!dir) return -1;

    struct dirent *entry;
    char linkTarget[256];
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_LNK) {
            std::string fullPath = fdPath + "/" + entry->d_name;
            ssize_t len = readlink(fullPath.c_str(), linkTarget, sizeof(linkTarget) - 1);
            if (len != -1) {
                linkTarget[len] = '\0';
                std::string target(linkTarget);
                if (target.find("socket:[") != std::string::npos) {
                    size_t start = target.find('[') + 1;
                    size_t end = target.find(']');
                    int inode = std::stoi(target.substr(start, end - start));
                    inodes.push_back(inode);
                }
            }
        }
    }
    closedir(dir);
    return 0;
}

void FetchNwStats(int pid, NwStats &netstats) {
    memset(&netstats, 0, sizeof(NwStats));
    std::vector<int> processInodes;
    std::vector<SocketStats> tcpSockets;
    std::vector<SocketStats> tcp6Sockets;
    std::vector<SocketStats> udpSockets;
    std::vector<SocketStats> udp6Sockets;

    getProcessSocketInodes(pid, processInodes);

    parseNetFile("/proc/net/tcp", tcpSockets);
    parseNetFile("/proc/net/tcp6", tcp6Sockets);
    parseNetFile("/proc/net/udp", udpSockets);
    parseNetFile("/proc/net/udp6", udp6Sockets);

    unsigned long tcpTx = 0, tcpRx = 0;
    unsigned long udpTx = 0, udpRx = 0;

    for (int inode : processInodes) {
        for (const auto &sock : tcpSockets) {
            if (sock.inode == inode) {
                tcpTx += sock.tx_queue;
                tcpRx += sock.rx_queue;
            }
        }
        for (const auto &sock : tcp6Sockets) {
            if (sock.inode == inode) {
                tcpTx += sock.tx_queue;
                tcpRx += sock.rx_queue;
            }
        }
        for (const auto &sock : udpSockets) {
            if (sock.inode == inode) {
                udpTx += sock.tx_queue;
                udpRx += sock.rx_queue;
            }
        }
        for (const auto &sock : udp6Sockets) {
            if (sock.inode == inode) {
                udpTx += sock.tx_queue;
                udpRx += sock.rx_queue;
            }
        }
    }

    netstats.tcp_tx = tcpTx;
    netstats.tcp_rx = tcpRx;
    netstats.udp_tx = udpTx;
    netstats.udp_rx = udpRx;

    return;
}

// Read GPU memory usage from memstore
std::string FetchGpuStats(GpuStats &gpustats) {
    memset(&gpustats, 0, sizeof(GpuStats));
    std::string path = "/sys/class/kgsl/kgsl-3d0/";

    std::ifstream file(path);
    if (!file.is_open()) return "N/A";

    std::string line;
    unsigned long totalBytes = 0;
    while (std::getline(file, line)) {
        // memstore lines often contain: context_id size
        std::istringstream iss(line);
        int contextId;
        unsigned long size;
        if (iss >> contextId >> size) {
            totalBytes += size;
        }
    }
    return std::to_string(totalBytes) + " bytes";

    const char *devicePath = "/dev/kgsl-3d0";
    int fd = open(devicePath, O_RDWR);
    if (fd < 0) {
        syslog(LOG_ERR, "Failed to open %s: %m", devicePath);
        return "N/A";
    }

    struct kgsl_memstats memStats;
    memset(&memStats, 0, sizeof(memStats));

    // IOCTL to query memory stats
    if (ioctl(fd, KGSL_IOC_QUERY_MEMSTAT, &memStats) < 0) {
        syslog(LOG_ERR, "KGSL IOCTL failed: %m");
        close(fd);
        return "N/A";
    }

    gpustats.mem_total = memStats.gpu_mem_total;
    gpustats.mem_allocated = memStats.gpu_mem_allocated;
    gpustats.mem_free = memStats.gpu_mem_free;

    struct kgsl_gpu_busy busy;
    memset(&busy, 0, sizeof(busy));

    if (ioctl(fd, KGSL_IOC_GPU_BUSY, &busy) < 0) {
        syslog(LOG_ERR, "KGSL IOCTL failed: %m");
        close(fd);
        return "N/A";
    }

    gpustats.busy_percent = (busy.busy_time && busy.total_time)
                         ? (double)busy.busy_time / busy.total_time * 100.0
                         : 0.0;

    close(fd);
    return 0;
}

int getActiveDisplays(std::vector<std::string> &activeDisplays) {
    int count = 0;
    DIR *dir = opendir("/sys/class/drm/");
    if (!dir) return 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name(entry->d_name);
        if (name.find("card") != std::string::npos) {
            std::string statusPath = "/sys/class/drm/" + name + "/status";
            struct stat st;
            if (stat(statusPath.c_str(), &st) == 0) { // Check if status file exists
                std::ifstream file(statusPath);
                if (file.is_open()) {
                    std::string status;
                    file >> status;
                    if (status == "connected") {
                        activeDisplays.push_back(name);
                        count++;
                    }
                }
            }
        }
    }
    closedir(dir);
    return count;
}

void FetchDisplayStats(DispStats &dispstats) {
    dispstats.display_on = 0;

    // 1. Check backlight (optional)
    std::string backlightPath = "/sys/class/backlight/";
    DIR *dir = opendir(backlightPath.c_str());
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_name[0] != '.') {
                std::string filePath = backlightPath + entry->d_name + "/bl_power";
                std::ifstream file(filePath);
                if (file.is_open()) {
                    int value;
                    file >> value;
                    if (value == 0) {
                        dispstats.display_on = 1;
                        break;
                    }
                }
            }
        }
        closedir(dir);
    }

    // 2. Check DRM connectors if display_on still 0
    if (dispstats.display_on == 0) {
        DIR *drmDir = opendir("/sys/class/drm/");
        if (drmDir) {
            struct dirent *entry;
            while ((entry = readdir(drmDir)) != nullptr) {
                std::string name(entry->d_name);
                if (name.find("card") != std::string::npos) {
                    std::string statusPath = "/sys/class/drm/" + name + "/status";
                    struct stat st;
                    if (stat(statusPath.c_str(), &st) == 0) { // status file exists
                        std::ifstream statusFile(statusPath);
                        if (statusFile.is_open()) {
                            std::string status;
                            statusFile >> status;
                            if (status == "connected") {
                                dispstats.display_on = 1;
                                break;
                            }
                        }
                    }
                }
            }
            closedir(drmDir);
        }
    }
}

void read_schedstat(pid_t pid, SchedStats &ss) {
    memset(&ss, 0, sizeof(SchedStats));
    std::string path = "/proc/" + std::to_string(pid) + "/schedstat";
    std::ifstream in(path);
    if (!in.is_open()) return;
    std::string content;
    std::getline(in, content);
    std::istringstream iss(content);

    // Common format: 3 fields; newer kernels may add more
    if (!(iss >> ss.runtime_ns >> ss.rq_wait_ns >> ss.timeslices)) {
        return; // unexpected format
    }
    return;
}
