#ifndef INFERENCE_H
#define INFERENCE_H

#include "ContextualClassifier.h"
#include <cstdint>
#include <map>
#include <string>

class Inference {
public:
    explicit Inference(const std::string &model_path) : model_path_(model_path) {}
    virtual ~Inference() = default;

    // Base virtual API: returns 0 on success, non-zero on failure.
    // 'cat' is set to the predicted category (or "Unknown" in the base).
    virtual CC_TYPE Classify(int pid) {
    (void)pid;
    // Base implementation: no ML, just return default".
    return CC_APP;
}

protected:
    std::string model_path_;
};

#endif // INFERENCE_H
