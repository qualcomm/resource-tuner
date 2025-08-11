# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import os
import sys
import signal
import time
import logging
from multiprocessing import Process

class SystuneServerLifecycleTests:
    def __init__(self):
        pass

    def scanPIDsToFindChild(self, pid):
        childCreated = False
        for filename in os.listdir("/proc/"):
            try:
                with open("/proc/" + str(filename) + "/status") as statusFile:
                    lines = statusFile.readlines()
                    for line in lines:
                        lineData = line.split(":")
                        if lineData[0].strip() == "PPid":
                            if int(lineData[1].strip()) == pid:
                                childCreated = True
                            break
            except (FileNotFoundError, ValueError, NotADirectoryError) as e:
                pass

        return childCreated

    def TestSystuneServerBootupAndTermination(self):
        logger = logging.getLogger()
        logger.info(self.TestSystuneServerBootupAndTermination.__name__ + ": STARTED")
        testResult = True

        pid = os.fork()
        if pid == 0:
            time.sleep(1)
            os.execvp("./systune", ["./systune", "--start"])
        elif pid > 0:
            time.sleep(3)
            if os.path.isfile('/proc/' + str(pid) + '/status') == False:
                logger.error('Server Initialization failed')
                testResult = False
            else:
                logger.info('Server Started Successfully')
                # Check if Child Process is created
                childCreated = self.scanPIDsToFindChild(pid)

                if childCreated == False:
                    testResult = False
                    logger.error('Child Daemon Process Creation Failed')

                os.kill(pid, signal.SIGINT)
                os.wait()
                if os.path.isfile('/proc/' + str(pid) + '/status') == False:
                    logger.info('Server Process Successfully Terminated')
                else:
                    logger.error('Server shutdown failed')
                    testResult = False
        else:
            logger.error('Child Process Creation Failed, Aborting')
            testResult = False

        if testResult == True:
            logger.info(self.TestSystuneServerBootupAndTermination.__name__ + ": PASSED")

        return testResult

    def TestSystuneServerCrashRecovery(self):
        logger = logging.getLogger()
        logger.info(self.TestSystuneServerCrashRecovery.__name__ + ": STARTED")
        testResult = True

        pid = os.fork()
        if pid == 0:
            time.sleep(1)
            os.execvp("./systune", ["./systune", "--start"])
        elif pid > 0:
            time.sleep(3)
            if os.path.isfile('/proc/' + str(pid) + '/status') == False:
                logger.error('Server Initialization failed')
                testResult = False
            else:
                logger.info('Server Started Successfully')

                # Check that the sysfsOriginalValues.txt (Crash Recovery) file is created
                if os.path.isfile('../sysfsOriginalValues.txt') == False:
                    logger.error('Crash Recovery Data File creation Failed')
                    testResult = False
                else:
                    logger.info('Crash Recovery Data File Successfully created')

                os.kill(pid, signal.SIGINT)
                os.wait()
                if os.path.isfile('/proc/' + str(pid) + '/status') == False:
                    logger.info('Server Process Successfully Terminated')

                    # Verify that the Crash Recovery Data file has been deleted
                    if testResult:
                        if os.path.isfile('../sysfsOriginalValues.txt') == False:
                            logger.info('Crash Recovery Data File successfully Deleted')
                        else:
                            logger.error('Crash Recovery Data File Deletion Failed')
                            testResult = False
                else:
                    logger.error('Server shutdown failed')
                    testResult = False
        else:
            logger.error('Child Process Creation Failed, Aborting')
            testResult = False

        if testResult == True:
            logger.info(self.TestSystuneServerCrashRecovery.__name__ + ": PASSED")

        return testResult


if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s [%(levelname)s] %(message)s",
        datefmt="[%Y-%m-%d %H:%M:%S]",
        handlers=[logging.StreamHandler(sys.stdout)],
    )

    testObj = SystuneServerLifecycleTests()

    assert testObj.TestSystuneServerBootupAndTermination() == True, "Test: TestSystuneServerBootupAndTermination Failed"
    assert testObj.TestSystuneServerCrashRecovery() == True, "Test: TestSystuneServerCrashRecovery Failed"

    logger = logging.getLogger()
    logger.info("All Tests part of the SystuneServerLifecycleTests suite, Successfully Passed")
