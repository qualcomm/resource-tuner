#include "Inference.h"

uint32_t Inference::predict(int pid,
                            const std::map<std::string, std::string> &raw_data,
                            std::string &cat) {
    (void)pid;
    (void)raw_data;
    // Base implementation: no ML, just return "Unknown".
    cat = "Unknown";
    return 0;
}
