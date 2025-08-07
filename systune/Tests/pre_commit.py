# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import os
import signal
import time
import sys

# Unit tests
def unit_tests():
    # os.system("./Tests/Unit/TimerTest")
    os.system("./Tests/Unit/SafeOpsTest")
    os.system("./Tests/Unit/MiscTests")
    # os.system("./Tests/Unit/ThreadPoolTests")
    # os.system("./Tests/Unit/MemoryPoolTests")
    # os.system("./Tests/Unit/RequestMapTests")
    os.system("./Tests/Unit/ResourceProcessorTests")
    os.system("./Tests/Unit/SysSignalConfigProcessorTests")
    os.system("./Tests/Unit/SysConfigAPITests")
    os.system("./Tests/Unit/SysConfigProcessorTests")
    os.system("./Tests/Unit/TargetConfigProcessorTests")
    os.system("./Tests/Unit/ClientDataManagerTests")
    # os.system("./Tests/Unit/RateLimiterTests")

# System Tests
def system_tests():
    pid = os.fork()
    if pid == 0:
        # Start the Server
        os.execvp("./systune", ["./systune", "--test"])
    elif pid > 0:
        time.sleep(4)
        os.system("./sys_tests_ex")
    else:
        print("System Wide Tests cannot be run as Server startup failed")

    # Terminate the Server
    time.sleep(2)
    os.kill(pid, signal.SIGINT)
    time.sleep(2)

# Server Lifecycle Tests
def lifecycle_tests():
    os.system("python ../Tests/System/ServerLifecyleTests.py")

if __name__ == "__main__":
    unit_tests()
    # system_tests()
    # lifecycle_tests()
