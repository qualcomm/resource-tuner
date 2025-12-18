// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef ML_INFERENCE_H
#define ML_INFERENCE_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <mutex>
#include <fasttext/fasttext.h>

class MLInference {
public:
    MLInference(const std::string& ft_model_path);
    ~MLInference();

    std::string predict(int pid, const std::map<std::string, std::string>& raw_data);

private:
    fasttext::FastText ft_model_;
    std::mutex predict_mutex_;

    std::vector<std::string> classes_;
    std::vector<std::string> text_cols_;
    int embedding_dim_;

    std::string normalize_text(const std::string& text);

public: // Public getters for feature lists
    const std::vector<std::string>& getTextCols() const { return text_cols_; }
};

#endif // ML_INFERENCE_H
