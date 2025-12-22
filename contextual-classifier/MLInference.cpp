// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "MLInference.h"
#include <algorithm> // Add this include for std::transform
#include <cmath>     // For std::exp
#include <fstream>   // Add this include for std::ifstream
#include <iomanip>   // For std::fixed and std::setprecision
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept> // Add this include for std::runtime_error
#include <string>
#include <syslog.h> // Include syslog for logging
#include <vector>

MLInference::MLInference(const std::string &ft_model_path) {
    text_cols_ = {"attr", "cgroup",  "cmdline", "comm", "maps",
                  "fds",  "environ", "exe",     "logs"};

    syslog(LOG_DEBUG, "Loading fastText model from: %s", ft_model_path.c_str());
    try {
        ft_model_.loadModel(ft_model_path);
        embedding_dim_ = ft_model_.getDimension();
        syslog(LOG_DEBUG, "fastText model loaded. Embedding dimension: %d",
               embedding_dim_);
    } catch (const std::exception &e) {
        syslog(LOG_CRIT, "Failed to load fastText model: %s", e.what());
        throw;
    }

    syslog(LOG_INFO, "MLInference initialized. fastText dim: %d",
           embedding_dim_);
}

MLInference::~MLInference() {}

std::string MLInference::normalize_text(const std::string &text) {
    std::string s = text;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::string
MLInference::predict(int pid,
                     const std::map<std::string, std::string> &raw_data) {
    std::lock_guard<std::mutex> lock(predict_mutex_);
    syslog(LOG_DEBUG, "Starting prediction.");

    std::string concatenated_text;
    for (const auto &col : text_cols_) {
        auto it = raw_data.find(col);
        if (it != raw_data.end()) {
            concatenated_text += normalize_text(it->second) + " ";
        } else {
            concatenated_text += " "; // Add space for missing text columns
        }
    }
    // Remove trailing space if any
    if (!concatenated_text.empty() && concatenated_text.back() == ' ') {
        concatenated_text.pop_back();
    }

    if (concatenated_text.empty()) {
        syslog(LOG_WARNING, "No text features found.");
        return "Unknown";
    }

    syslog(LOG_DEBUG, "Calling fastText predict().");

    // Add a newline to the end of the text as is typical for fastText stream
    // processing
    concatenated_text += "\n";
    std::istringstream iss(concatenated_text);

    std::vector<std::pair<fasttext::real, int>> predictions;

    // Tokenize the text using the dictionary
    std::vector<int> words, labels;
    ft_model_.getDictionary()->getLine(iss, words, labels);

    ft_model_.predict(1, words, predictions, 0.0);

    if (predictions.empty()) {
        syslog(LOG_WARNING, "fastText returned no predictions.");
        return "Unknown";
    }

    // fastText returns pairs of (probability, label_id)
    fasttext::real probability = predictions[0].first;
    if (probability < 0) {
        probability = std::exp(probability);
    }
    int label_id = predictions[0].second;

    // Get the label string from the dictionary
    std::string predicted_label = ft_model_.getDictionary()->getLabel(label_id);

    // Strip __label__ prefix if present
    std::string prefix = "__label__";
    if (predicted_label.rfind(prefix, 0) == 0) { // starts with prefix
        predicted_label = predicted_label.substr(prefix.length());
    }

    std::string comm = "unknown";
    if (raw_data.count("comm")) {
        comm = raw_data.at("comm");
    }

    syslog(
        LOG_INFO,
        "Prediction complete. PID: %d, Comm: %s, Class: %s, Probability: %.4f",
        pid, comm.c_str(), predicted_label.c_str(), probability);
    return predicted_label;
}
