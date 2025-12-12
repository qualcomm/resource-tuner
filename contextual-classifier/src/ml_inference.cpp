#include "ml_inference.h"
#include <iostream>
#include <sstream>
#include <fstream> // Add this include for std::ifstream
#include <algorithm> // Add this include for std::transform
#include <stdexcept> // Add this include for std::runtime_error
#include <syslog.h> // Include syslog for logging
#include <vector>
#include <string>
#include <map>
#include <iomanip> // For std::fixed and std::setprecision

// Simple JSON parser for meta.json (can be replaced by a proper library like nlohmann/json)
namespace json_parser {
    std::map<std::string, std::vector<std::string>> parse_meta(const std::string& filepath) {
        std::map<std::string, std::vector<std::string>> meta_data;
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            throw std::runtime_error("Could not open meta.json file: " + filepath);
        }
        std::string line;
        std::string content;
        while (std::getline(ifs, line)) {
            content += line;
        }

        // Basic parsing for specific keys. This is very fragile and should be replaced by a proper JSON library.
        // For example, to parse "classes": ["Game", "Browser", "Other"]
        size_t pos = content.find("\"classes\":");
        if (pos != std::string::npos) {
            size_t start = content.find("[", pos);
            size_t end = content.find("]", start);
            std::string classes_str = content.substr(start + 1, end - start - 1);
            std::stringstream ss(classes_str);
            std::string segment;
            while(std::getline(ss, segment, ',')) {
                segment.erase(std::remove(segment.begin(), segment.end(), ' '), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '\t'), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '\n'), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '\r'), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '"'), segment.end());
                if (!segment.empty()) {
                    meta_data["classes"].push_back(segment);
                }
            }
        }

        pos = content.find("\"text_cols\":");
        if (pos != std::string::npos) {
            size_t start = content.find("[", pos);
            size_t end = content.find("]", start);
            std::string cols_str = content.substr(start + 1, end - start - 1);
            std::stringstream ss(cols_str);
            std::string segment;
            while(std::getline(ss, segment, ',')) {
                segment.erase(std::remove(segment.begin(), segment.end(), ' '), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '\t'), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '\n'), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '\r'), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '"'), segment.end());
                if (!segment.empty()) {
                    meta_data["text_cols"].push_back(segment);
                }
            }
        }

        pos = content.find("\"numeric_cols\":");
        if (pos != std::string::npos) {
            size_t start = content.find("[", pos);
            size_t end = content.find("]", start);
            std::string cols_str = content.substr(start + 1, end - start - 1);
            std::stringstream ss(cols_str);
            std::string segment;
            while(std::getline(ss, segment, ',')) {
                segment.erase(std::remove(segment.begin(), segment.end(), ' '), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '\t'), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '\n'), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '\r'), segment.end());
                segment.erase(std::remove(segment.begin(), segment.end(), '"'), segment.end());
                if (!segment.empty()) {
                    meta_data["numeric_cols"].push_back(segment);
                }
            }
        }
        return meta_data;
    }
}

// Placeholder for actual fastText and LightGBM implementation
// This is a minimal implementation to satisfy the compilation for now.

// Helper to convert std::string to float, handling potential errors
float string_to_float(const std::string& s) {
    try {
        return std::stof(s);
    } catch (const std::invalid_argument& e) {
        return 0.0f; // Default to 0.0 or NaN, depending on desired behavior
    } catch (const std::out_of_range& e) {
        return 0.0f; // Default to 0.0 or NaN
    }
}

// Helper to log a vector of doubles for debugging
void log_double_vector(const std::vector<double>& vec, const char* name) {
    std::stringstream ss;
    ss << name << " (first 10 elements): [";
    for (size_t i = 0; i < std::min(vec.size(), (size_t)10); ++i) {
        ss << std::fixed << std::setprecision(4) << vec[i] << (i < std::min(vec.size(), (size_t)10) - 1 ? ", " : "");
    }
    ss << "]";
    syslog(LOG_DEBUG, "%s", ss.str().c_str());
}

MLInference::MLInference(const std::string& ft_model_path, const std::string& lgbm_model_path, const std::string& meta_path) : lgbm_booster_(nullptr) {
    syslog(LOG_DEBUG, "Parsing meta.json from: %s", meta_path.c_str());
    try {
        auto meta_data = json_parser::parse_meta(meta_path);
        classes_ = meta_data["classes"];
        text_cols_ = meta_data["text_cols"];
        numeric_cols_ = meta_data["numeric_cols"];
        syslog(LOG_DEBUG, "Meta.json parsed successfully. Found %zu classes.", classes_.size());
    } catch (const std::runtime_error& e) {
        syslog(LOG_CRIT, "Failed to parse meta.json: %s", e.what());
        throw;
    }

    syslog(LOG_DEBUG, "Loading fastText model from: %s", ft_model_path.c_str());
    try {
        ft_model_.loadModel(ft_model_path);
        embedding_dim_ = ft_model_.getDimension();
        syslog(LOG_DEBUG, "fastText model loaded. Embedding dimension: %d", embedding_dim_);
    } catch (const std::exception& e) {
        syslog(LOG_CRIT, "Failed to load fastText model: %s", e.what());
        throw;
    }

    syslog(LOG_DEBUG, "Loading LightGBM model from: %s", lgbm_model_path.c_str());
    int num_iterations;
    if (LGBM_BoosterCreateFromModelfile(lgbm_model_path.c_str(), &num_iterations, &lgbm_booster_) != 0) {
        syslog(LOG_CRIT, "Failed to load LightGBM model from file: %s", lgbm_model_path.c_str());
        throw std::runtime_error("Failed to load LightGBM model.");
    }

    int num_features = 0;
    if (LGBM_BoosterGetNumFeature(lgbm_booster_, &num_features) != 0) {
        syslog(LOG_CRIT, "Failed to get number of features from LightGBM model.");
        throw std::runtime_error("Failed to get number of features from LightGBM model.");
    }
    lgbm_expected_features_ = num_features;
    syslog(LOG_INFO, "MLInference initialized. LightGBM features: %d, fastText dim: %d", lgbm_expected_features_, embedding_dim_);
}

MLInference::~MLInference() {
    if (lgbm_booster_) {
        LGBM_BoosterFree(lgbm_booster_);
    }
}

std::string MLInference::normalize_text(const std::string& text) {
    std::string s = text;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    // Add other normalization steps as needed (e.g., replace newlines, remove special chars)
    // For now, mirroring _norm_text from Python:
    // s = s.replace('\n', ' ').replace('\t', ' ')
    // s = re.sub(r'[^A-Za-z0-9_:\-]+', ' ', s)
    return s;
}

std::vector<double> MLInference::get_feature_vector(const std::map<std::string, std::string>& raw_data) {
    syslog(LOG_DEBUG, "Starting feature vector creation.");
    std::vector<double> feature_vector(numeric_cols_.size() + embedding_dim_, 0.0);

    // 1. Numeric features
    for (size_t i = 0; i < numeric_cols_.size(); ++i) {
        const auto& col = numeric_cols_[i];
        auto it = raw_data.find(col);
        if (it != raw_data.end()) {
            feature_vector[i] = string_to_float(it->second);
        }
        // The vector is already initialized to 0.0, so no else is needed.
    }
    syslog(LOG_DEBUG, "Numeric features processed. Count: %zu", feature_vector.size());

    // 2. fastText embeddings
    std::string concatenated_text;
    for (const auto& col : text_cols_) {
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
    
    
    if (!concatenated_text.empty()) {
        syslog(LOG_DEBUG, "Generating fastText embedding.");
        // Add a newline to the end of the text, as fastText's getSentenceVector (with istream) expects it.
        concatenated_text += "\n";
        fasttext::Vector ft_embedding_vector(embedding_dim_);
        std::istringstream iss(concatenated_text);
        ft_model_.getSentenceVector(iss, ft_embedding_vector);
        for (int i = 0; i < embedding_dim_; ++i) {
            feature_vector[numeric_cols_.size() + i] = ft_embedding_vector[i];
        }
    } else {
        syslog(LOG_WARNING, "No text features found; embedding is already zeros.");
    }
    syslog(LOG_DEBUG, "Feature vector created. Total size: %zu", feature_vector.size());
    log_double_vector(feature_vector, "Final Feature Vector");

    return feature_vector;
}

std::string MLInference::predict(const std::map<std::string, std::string>& raw_data) {
    std::lock_guard<std::mutex> lock(predict_mutex_);
    syslog(LOG_DEBUG, "Starting prediction.");
    std::vector<double> features = get_feature_vector(raw_data);

    // Defensive check for feature count mismatch
    if (features.size() != static_cast<size_t>(lgbm_expected_features_)) {
        syslog(LOG_CRIT, "Feature mismatch! LGBM expects %d features, but got %zu.",
               lgbm_expected_features_, features.size());
        throw std::runtime_error("Feature vector size does not match LightGBM model expectation.");
    }
    
    // LightGBM prediction
    std::vector<double> result(classes_.size());
    int64_t out_len = 0;
    syslog(LOG_DEBUG, "Calling LightGBM C_API Predict().");

    if (LGBM_BoosterPredictForMat(lgbm_booster_,
                                  features.data(),
                                  C_API_DTYPE_FLOAT64,
                                  1, // Number of rows
                                  features.size(),
                                  1, // Is row-major
                                  C_API_PREDICT_NORMAL,
                                  -1, // start iteration
                                  -1, // num iteration
                                  "", // parameters
                                  &out_len,
                                  result.data()) != 0) {
        syslog(LOG_CRIT, "LightGBM prediction failed.");
        throw std::runtime_error("LightGBM prediction failed.");
    }

    log_double_vector(result, "LGBM Prediction Probabilities");

    // Get the predicted class index
    int predicted_class_idx = 0;
    double max_prob = -1.0;
    for (size_t i = 0; i < classes_.size(); ++i) {
        if (result[i] > max_prob) {
            max_prob = result[i];
            predicted_class_idx = i;
        }
    }

    syslog(LOG_INFO, "Prediction complete. Class: %s, Probability: %.4f", 
           classes_[predicted_class_idx].c_str(), max_prob);
    return classes_[predicted_class_idx];
}
