# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import os
import signal
import time
import sys
import subprocess
import logging
from pathlib import Path

def setup():
    logger = logging.getLogger()
    logger.info("Setting up CMake build directory")

    subprocess.run(["rm", "-rf", "build/"], check=True)
    os.mkdir("build")
    os.chdir("build")

    subprocess.run(["cmake", "..", "-DBUILD_SIGNALS=ON", "-DBUILD_TESTS=ON", "-DBUILD_CLI=ON"], check=True)
    subprocess.run(["cmake", "--build", "."], check=True)

    currDirPath = os.getcwd()
    configDirPath = str(Path(__file__).parent) + "/Configs"

    for fileName in configFilesToCopy:
        # Copy files to current directory
        os.system("cp " + configDirPath +  "/" + fileName + " " + currDirPath)

    logger.info("Setup Completed, Test Build is ready")

def cleanup():
    logger = logging.getLogger()
    logger.info("Proceeding with build cleanup")

    for fileName in configFilesToCopy:
        # Delete Copies files from current directory
        if os.path.exists(fileName):
            os.remove(fileName)

    os.chdir("..")
    subprocess.run(["rm", "-rf", "build/"], check=True)

    logger.info("Cleanup Completed")

def check_for_failures(log_file_path="../tests_run_log.txt"):
    with open(log_file_path, 'r') as file:
        for line in file:
            if "FAILED" in line:
                return True
    return False

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

    logger.info("All Unit Tests Ran Successfully")
    return True

# System Tests
def system_tests():
    logger = logging.getLogger()
    logger.info("Running System Wide Tests")

    pid = os.fork()
    if pid == 0:
        # Start the Server
        os.execvp("./resource_tuner", ["./resource_tuner", "--test"])
    elif pid > 0:
        time.sleep(4)
        if not run_test(["./resource_tuner_tests"]):
            return False
    else:
        return False

    # Terminate the Server
    time.sleep(2)
    os.kill(pid, signal.SIGINT)
    time.sleep(2)

    logger.info("All System Wide Tests Ran Successfully")
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
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s [%(levelname)s] %(message)s",
        datefmt="[%Y-%m-%d %H:%M:%S]",
        handlers=[logging.StreamHandler(sys.stdout)],
    )

    logger = logging.getLogger()

    # Create the build directory
    setup()
    time.sleep(5)

    # Run the Tests
    if not unit_tests():
        logger.error("Unit tests failed, Exiting process.")
        cleanup()
        sys.exit(1)

    if not system_tests():
        logger.error("System Wide Tests cannot be run as Server startup failed, Exiting process")
        cleanup()
        sys.exit(1)

    if not lifecycle_tests():
        logger.error("Server Lifecycle tests failed. Exiting process.")
        cleanup()
        sys.exit(1)

    if check_for_failures() == True:
        logger.error('One or more tests failed, Exiting process')
        cleanup()
        sys.exit(1)

    time.sleep(5)

    logger.info("All Resource Tuner Tests Successfully Passed")

    cleanup()
