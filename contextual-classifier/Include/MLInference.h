// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef ML_INFERENCE_H
#define ML_INFERENCE_H

#include "Inference.h"
#include <vector>

#ifdef USE_FASTTEXT
#include <fasttext/fasttext.h>
#include <mutex>
#include <string>
#endif

class MLInference : public Inference {
  public:
    MLInference(const std::string &ft_model_path);
    ~MLInference();

    // Derived implementation using fastText.
    // Returns 0 on success, non-zero on failure and sets 'cat' accordingly.
    uint32_t predict(int pid,
                     const std::map<std::string, std::string> &raw_data,
                     std::string &cat) override;

  private:
#ifdef USE_FASTTEXT
    fasttext::FastText ft_model_;
    std::mutex predict_mutex_;

    std::vector<std::string> classes_;
    std::vector<std::string> text_cols_;
    int embedding_dim_;
#endif

    std::string normalize_text(const std::string &text);

  public: // Public getters for feature lists
#ifdef USE_FASTTEXT
    const std::vector<std::string> &getTextCols() const { return text_cols_; }
#else
    const std::vector<std::string> &getTextCols() const {
        static const std::vector<std::string> empty;
        return empty;
    }
#endif
};

#endif // ML_INFERENCE_H
