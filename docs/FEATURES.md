\page ext_features Features Guide

Refer Relay.cpp in Examples section, for basic code-guidance.

# 1. Making Use of Relay Signals and Features
Resource Tuner allows requests to be forwarded to user-defined features (or functionality) via the use of Features and Relay Signals.

- Features are a set of predefined functionality registered with Resource Tuner.
- A Signal (SigCode) can be mapped to one or more such features
- When a relaySignal API request is issued for such a Signal, then the Request is forwarded to the pre-defined feature. Where the User can define any custom functionality.

To register features, make use of the ExtFeaturesConfig.yaml file. For example, a sample feature might look like:

```yaml
FeatureConfigs:
  - FeatId: "0x00000001"
    Name: "FEAT-1"
    LibPath: "/usr/lib/libtesttuner.so"
    Description: "Simple Algorithmic Feature, defined by the BU"
    Signals: ["0x000dbbca", "0x000a00ff"]
```

When a relaySignal API request is received for either Signal with the SigCode: 0x000dbbca or 0x000a00ff, then the Request shall be forwarded to the feature defined via the shared lib: /usr/lib/libtesttuner.so

---

# 2. Defining feature init, relay and teardown capabilities
Resoruce Tuner exposes 3 hooks for any feature registered:
    - initFeature: Use this function to initialize your feature
    - tearFeature: Use this function to cleanup your feature
    - relayFeature: This is the main hook, that is called when a relaySignal API request is received for any of your registered features.

In the above example, the lib: "/usr/lib/libtesttuner.so" is required to provide implementations for all of these hooks.

Refer Relay.cpp in Examples Section for more details on this, and the expected signature of these functions.

---

# 3. Where should the ExtFeaturesConfig.yaml be placed?
Since this config file is not common across targets, hence Resource Tuner does not provide a Common (or Base) version of this file (like it does for say Resources or Properties).

If you would like to register your own Features, then you need to provide your own version of this file. To register the file with Resource Tuner one of the following 2 strategies can be followed:
- Keep the file in /etc/resource-tuner/custom
- Or, If you would like more flexibility in terms of File Placement, then you can make use of the Extension Interface's RESTUNE_REGISTER_CONFIG macro, to notify Resource Tuner where the file would be placed.

For example:

```cpp
RESTUNE_REGISTER_CONFIG(EXT_FEATURES_CONFIG, "/opt/custom/ExtFeaturesConfig.yaml")
```

---
