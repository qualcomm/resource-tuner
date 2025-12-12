
"""
===============================================================================
Contextual Classifier Inference Script (fastText + LightGBM)
===============================================================================

Overview:
    This script performs inference for the User Contextual Classifier 
    by combining:
      - fastText sentence embeddings for textual fields
      - LightGBM booster for classification

    It loads metadata (feature schema) and pre-trained models, constructs a
    feature vector from raw process/resource data (mimicking C++/ProcFS
    collection), and prints the predicted class and probabilities.

Directory Structure (expected):
    ./artifacts/
        fasttext_model.bin   # fastText model
        lgbm_model.txt       # LightGBM Booster model
        meta.json            # Schema and class labels
    inference.py             # (this script)

Prerequisites:
    - Python 3.8+
    - pip install fasttext lightgbm numpy
    - Standard libraries used: json, re

meta.json keys (required):
    - text_cols: list[str]      # text fields to embed via fastText
    - numeric_cols: list[str]   # numeric features used directly
    - embedding_dim: int        # must equal fastText model dimension
    - classes: list[str]        # ordered labels for output

Feature Layout:
    [0 : len(numeric_cols))         -> numeric features (float)
    [len(numeric_cols) : end)       -> fastText sentence embedding
    Total features = len(numeric_cols) + embedding_dim

Text Normalization (Python):
    - Lowercase
    - Replace newlines and tabs with spaces
    - Optional regex for special characters (keep consistent with C++ pipeline)

Usage:
    1) Place required files in ./artifacts/
       - fasttext_model.bin
       - lgbm_model.txt
       - meta.json
    2) Run:
       python inference.py

Expected Output (on success):
    - Loaded metadata summary
    - Model load confirmations
    - Feature vector size and preview
    - Predicted probabilities and top label with confidence

"""

import fasttext
import lightgbm as lgb
import json
import numpy as np
import re

# --- Configuration paths (adjust if necessary) ---
MODEL_DIR = './artifacts/' # Assuming script runs from CWD
FASTTEXT_MODEL_PATH = MODEL_DIR + 'fasttext_model.bin'
LGBM_MODEL_PATH = MODEL_DIR + 'lgbm_model.txt'
META_PATH = MODEL_DIR + 'meta.json'

# --- 1. Load meta.json to get feature names ---
try:
    with open(META_PATH, 'r') as f:
        meta_data = json.load(f)
except FileNotFoundError:
    print(f"Error: meta.json not found at {META_PATH}")
    exit(1)

text_cols = meta_data['text_cols']
numeric_cols = meta_data['numeric_cols']
embedding_dim = meta_data['embedding_dim']
classes = meta_data['classes']

print(f"Loaded meta.json: {len(numeric_cols)} numeric_cols, {len(text_cols)} text_cols, embedding_dim={embedding_dim}, classes={classes}")
print(f"Expected total features for LightGBM: {len(numeric_cols) + embedding_dim}")

# --- 2. Load models ---
try:
    print(f"Loading fastText model from: {FASTTEXT_MODEL_PATH}")
    ft_model = fasttext.load_model(FASTTEXT_MODEL_PATH)
    print(f"fastText model loaded. Dimension: {ft_model.get_dimension()}")
except ValueError as e:
    print(f"Error loading fastText model: {e}. Ensure the model file is valid.")
    exit(1)
except Exception as e:
    print(f"An unexpected error occurred loading fastText model: {e}")
    exit(1)

try:
    print(f"Loading LightGBM model from: {LGBM_MODEL_PATH}")
    # LightGBM loads models from file directly
    lgbm_model = lgb.Booster(model_file=LGBM_MODEL_PATH)
    print(f"LightGBM model loaded.")
except lgb.basic.LightGBMError as e:
    print(f"Error loading LightGBM model: {e}. This might be due to a corrupted file or version mismatch.")
    exit(1)
except Exception as e:
    print(f"An unexpected error occurred loading LightGBM model: {e}")
    exit(1)

# --- 3. Prepare dummy raw data (mimicking C++ data collection) ---
# Create a dummy raw_data dictionary. In a real scenario, this would come from ProcFS parsing.
dummy_raw_data = {
    "cpu_time": "10.5", "threads": "2", "rss": "10240", "vms": "20480",
    "mem_vmpeak": "25000", "mem_vmlck": "0", "mem_hwm": "8000", "mem_vm_rss": "10000",
    "mem_vmsize": "22000", "mem_vmdata": "15000", "mem_vmstk": "100", "mem_vm_exe": "500",
    "mem_vmlib": "2000", "mem_vmpte": "50", "mem_vmpmd": "20", "mem_vmswap": "0",
    "mem_thread": "2", "read_bytes": "1000", "write_bytes": "500", "tcp_tx": "10",
    "tcp_rx": "20", "udp_tx": "5", "udp_rx": "8", "gpu_busy": "0.1",
    "gpu_mem_allocated": "100", "display_on": "1", "active_displays": "1",
    "runtime_ns": "1000000", "rq_wait_ns": "50000", "timeslices": "10",
    "attr": "system_u:system_r:test_t:s0",
    "cgroup": "0::/user.slice",
    "cmdline": "/usr/bin/example_app --flag val",
    "comm": "example_app",
    "maps": "/usr/lib/libexample.so",
    "fds": "/dev/null socket:[12345]",
    "environ": "PATH=/usr/bin HOME=/root",
    "exe": "/usr/bin/example_app",
    "logs": "Jan 01 00:00:01 example_app: started"
}

# --- 4. Function to normalize text (mimicking C++ normalize_text) ---
def normalize_text_py(text):
    s = text.lower()
    s = s.replace('\n', ' ').replace('\t', ' ')
    # Add other normalization steps as needed (e.g., remove special chars)
    # s = re.sub(r'[^A-Za-z0-9_:\-]+', ' ', s) # Uncomment if this regex is applied in C++
    return s

# --- 5. Function to get feature vector (mimicking C++ get_feature_vector) ---
def get_feature_vector_py(raw_data, ft_model, numeric_cols, text_cols, embedding_dim):
    feature_vector = np.zeros(len(numeric_cols) + embedding_dim, dtype=np.float64)

    # Numeric features
    for i, col in enumerate(numeric_cols):
        if col in raw_data:
            try:
                feature_vector[i] = float(raw_data[col])
            except (ValueError, TypeError):
                feature_vector[i] = 0.0 # Default for parsing errors or non-numeric values
        else:
            feature_vector[i] = 0.0 # Default for missing numeric columns


    # fastText embeddings
    concatenated_text = []
    for col in text_cols:
        if col in raw_data:
            concatenated_text.append(normalize_text_py(raw_data[col]))
        else:
            concatenated_text.append("") # Add empty string for missing text columns

    concatenated_text_str = " ".join(concatenated_text).strip()

    if concatenated_text_str:
        # fastText get_sentence_vector returns a numpy array
        ft_embedding = ft_model.get_sentence_vector(concatenated_text_str)
        feature_vector[len(numeric_cols):] = ft_embedding
    # else: fastText embedding part of vector is already zeros from initialization

    return feature_vector

# --- 6. Generate feature vector ---
print("\nGenerating feature vector in Python...")
feature_vec = get_feature_vector_py(dummy_raw_data, ft_model, numeric_cols, text_cols, embedding_dim)
print(f"Generated feature vector size: {len(feature_vec)}")
print(f"Generated feature vector (first 10 elements): {feature_vec[:10]}")

# --- 7. Perform prediction ---
print("\nPerforming LightGBM prediction in Python...")
try:
    # LightGBM expects a 2D array for prediction (even for a single sample)
    prediction_probs = lgbm_model.predict(feature_vec.reshape(1, -1))
    
    # Get predicted class index
    predicted_class_idx = np.argmax(prediction_probs)
    predicted_label = classes[predicted_class_idx]
    max_prob = prediction_probs[0, predicted_class_idx]

    print(f"Prediction successful!")
    print(f"Predicted probabilities: {prediction_probs}")
    print(f"Predicted label: {predicted_label}, Probability: {max_prob:.4f}")

except Exception as e:
    print(f"Error during LightGBM prediction in Python: {e}")
    print("This might indicate an issue with the model file, feature data, or LightGBM version incompatibility.")

print("\nPython inference script finished.")

