# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import os
import signal
import time
import sys
import subprocess
import logging
from pathlib import Path

def run_test(command):
    logger = logging.getLogger()
    logger.info(f"Running Test: {' '.join(command)}")

    result = subprocess.run(command)
    if result.returncode != 0:
        logger.error(f"Test failed: {' '.join(command)}")
        return False

    logger.info(f"Run Successful: {' '.join(command)}")
    return True

# Unit tests
def unit_tests():
    logger = logging.getLogger()
    logger.info("Running Unit Tests")

    if not run_test(["./Tests/Unit/Build/TimerTest"]):
        return False
    if not run_test(["./Tests/Unit/Build/YamlParserTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/SafeOpsTest"]):
        return False
    if not run_test(["./Tests/Unit/Build/MiscTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/ThreadPoolTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/MemoryPoolTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/RequestMapTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/ResourceParsingTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/ResourceProcessorTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/SignalParsingTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/SysSignalConfigProcessorTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/SysConfigAPITests"]):
        return False
    if not run_test(["./Tests/Unit/Build/SysConfigProcessorTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/TargetConfigProcessorTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/ClientDataManagerTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/RateLimiterTests"]):
        return False
    if not run_test(["./Tests/Unit/Build/ExtensionIntfTests"]):
        return False

    return True

# System Tests
def system_tests():
    if not run_test(["/usr/bin/resource_tuner_tests"]):
        return False
    return True

# Server Lifecycle Tests
def lifecycle_tests():
    logger = logging.getLogger()
    logger.info("Running Server Lifecycle Tests")

    if not run_test(["python", "../Tests/System/ServerLifecycleTests.py"]):
        return False

    logger.info("All Server Lifecycle Tests Ran Successfully")
    return True

if __name__ == "__main__":
    for _ in range(20):
        unit_tests():

    for _ in range(40):
        system_tests()
