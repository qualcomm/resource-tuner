// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Logger.h"

int8_t Logger::mCurrentLevel = DEBUG;
int8_t Logger::mLevelSpecificLogging = false;
RedirectOptions Logger::mRedirectOutputTo = LOG_FILE;

void Logger::configure(int8_t level, int8_t levelSpecificLogging, RedirectOptions redirectOutputTo) {
    mCurrentLevel = level;
    mLevelSpecificLogging = levelSpecificLogging;
    mRedirectOutputTo = redirectOutputTo;
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    time_t now_c = std::chrono::system_clock::to_time_t(now);
    tm local_tm = *localtime(&now_c);

    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local_tm);
    return std::string(buffer);
}

std::string Logger::levelToString(LogLevel level) {
    switch(level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case ERROR:
            return "ERROR";
        default:
            break;
    }
    return "";
}

void Logger::log(LogLevel level, const std::string& tag, const std::string& funcName, const std::string& message) {
    if(mLevelSpecificLogging) {
        if(level != mCurrentLevel) return;
    } else {
        if(level < mCurrentLevel) return;
    }

    std::string timestamp = getTimestamp();
    std::string levelStr = levelToString(level);

    if(mRedirectOutputTo == RedirectOptions::LOG_FILE) {
        std::ofstream logFile("log.txt", std::ios::app); //TODO: FIXME
        if(logFile.is_open()) {
            logFile << "[" << timestamp << "] [" << tag << "] [" << levelStr << "] " << funcName <<" "<< message << std::endl;
            logFile.close();
        }
    } else if(mRedirectOutputTo == RedirectOptions::FTRACE) {
        std::ofstream traceFile("/sys/kernel/debug/tracing/trace_marker", std::ios::app);
        if (traceFile.is_open()) {
            traceFile << "[" << timestamp << "] [" << tag << "] [" << levelStr << "] " << funcName <<" "<< message << std::endl;
            traceFile.close();
        }
    }
}

void Logger::typeLog(CommonMessageTypes type, const std::string& funcName, ...) {
    char buffer[128];
    va_list args;
    va_start(args, funcName);

    switch(type) {
        case CommonMessageTypes::NOTIFY_MODULE_ENABLED:
            vsnprintf(buffer, sizeof(buffer),
                     "Module: [%s] is Enabled, Proceeding with Initialization", args);

            Logger::log(ERROR, "RTN_CLIENT_DATA_MANAGER",
                        funcName, std::string(buffer));
            break;

        case CommonMessageTypes::CLIENT_ALLOCATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                     "Memory allocation for Client: " \
                     "[PID: %d, TID: %d]. Failed with error: %s", args);

            Logger::log(ERROR, "RTN_CLIENT_DATA_MANAGER",
                        funcName, std::string(buffer));
            break;

        case CommonMessageTypes::PROPERTY_RETRIEVAL_FAILED:
            Logger::log(ERROR, "RTN_SERVER_INIT", funcName,
                        "Failed to Fetch Properties, " \
                        "Boot Configs, Resource Tuner Server Initialization Failed.");
            break;

        case CommonMessageTypes::META_CONFIG_PARSE_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                     "Fetch Meta Configs failed, with error %s", args);

            Logger::log(ERROR, "RTN_SERVER_INIT", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_CREATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Failed to create Thread Pool. Error: %s", args);

            Logger::log(ERROR, "RTN_SERVER_INIT", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_INIT_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Could Not Initialize Thread Pool. Error: %s", args);

            Logger::log(ERROR, "RTN_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_THREAD_ALLOCATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Failed to allocate Memory for ThreadNode. " \
                      "Error: %s", args);

            Logger::log(ERROR, "RTN_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_THREAD_CREATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Failed to create New thread, call to std::thread Failed. " \
                      "Error: %s", args);

            Logger::log(ERROR, "RTN_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_THREAD_TERMINATED:
            vsnprintf(buffer, sizeof(buffer),
                      "Thread Terminated with error: %s", args);

            Logger::log(ERROR, "RTN_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_ENQUEUE_TASK_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Task Submisission failed with error: %s", args);

            Logger::log(ERROR, "RTN_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_FULL_ALERT:
            Logger::log(ERROR, "RTN_THREAD_POOL", funcName,
                        "ThreadPool is full, Task Submission Failed");

            break;

        case CommonMessageTypes::NOTIFY_RESOURCE_TUNER_INIT_START:
            Logger::log(INFO, "RTN_SERVER_INIT", funcName,
                        "Starting Resource Tuner Server initialization");

            break;

        case CommonMessageTypes::RESOURCE_TUNER_DAEMON_CREATION_FAILURE:
            Logger::log(ERROR, "RTN_SERVER_INIT", funcName,
                        "Failed to create Resource Tuner Daemon, " \
                        "Aborting Initialization.");
            break;

        case CommonMessageTypes::PULSE_MONITOR_INIT_FAILED:
            Logger::log(ERROR, "RTN_SERVER_INIT", funcName,
                        "Pulse Monitor Could not be started, " \
                        "Aborting Initialization.");
            break;

        case CommonMessageTypes::GARBAGE_COLLECTOR_INIT_FAILED:
            Logger::log(ERROR, "RTN_SERVER_INIT", funcName,
                        "Client Garbage Collector Could not be started, " \
                        "Aborting Initialization.");
            break;

        case CommonMessageTypes::MEMORY_POOL_INVALID_BLOCK_SIZE:
            vsnprintf(buffer, sizeof(buffer),
                     "Invalid block Size Provided [size = %d bytes.]", args);

            Logger::log(ERROR, "RTN_MEMORY_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "No Free Blocks of Requested Size Available " \
                      "[size = %d bytes.]", args);

            Logger::log(ERROR, "RTN_MEMORY_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::MEMORY_POOL_ALLOCATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Insufficient Memory to Allocate Desired Number of Blocks" \
                      "[Requested Size = %d bytes, "                             \
                      "Requested Count = %d, "                                   \
                      "Allocated Count = %d.]", args);

            Logger::log(ERROR, "RTN_MEMORY_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::MODULE_INIT_FAILED:
            vsnprintf(buffer, sizeof(buffer),
                      "Failed to Initialize Module: %s, " \
                      "Aborting Server Startup.", args);

            Logger::log(ERROR, "RTN_SERVER_INIT", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::REQUEST_MEMORY_ALLOCATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Memory allocation for Incoming Request. " \
                      "Failed with Error: %s", args);

            Logger::log(ERROR, "RTN_REQUEST_ALLOCATION_FAILURE",
                        funcName, std::string(buffer));
            break;

        case CommonMessageTypes::REQUEST_PARSING_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Request Parsing Failed, Request is Malformed. " \
                      "Error: %s", args);

            Logger::log(ERROR, "RTN_REQUEST_PARSING_FAILURE",
                        funcName, std::string(buffer));
            break;

        case CommonMessageTypes::ERRNO_LOG:
            vsnprintf(buffer, sizeof(buffer),
                      "Call to %s, Failed with Error: %s", args);

            Logger::log(ERROR, "RTN_SYSCALL_FAILURE", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_INVALID_OPCODE:
            vsnprintf(buffer, sizeof(buffer),
                      "Invalid Opcode [%u], Dropping Request.", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_INVALID_PERMISSION:
            vsnprintf(buffer, sizeof(buffer),
                      "Permissions for Client [PID: %d, TID: %d] Could not be Fetched, " \
                      "Dropping Request.", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_INVALID_PRIORITY:
            vsnprintf(buffer, sizeof(buffer),
                      "Invalid Priority [%d], Dropping Request.", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_UNSUPPORTED_SIGNAL_TUNING:
            vsnprintf(buffer, sizeof(buffer),
                      "Specified Signal [%u] is not enabled for provisioning", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_VALUE_OUT_OF_BOUNDS:
            vsnprintf(buffer, sizeof(buffer),
                      "Config Value [%d] does not fall in the Allowed Range" \
                      "for the Resource [%u], Dropping Request.", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_NOT_SUFFICIENT_PERMISSION:
            vsnprintf(buffer, sizeof(buffer),
                      "Permission Check Failed for Resource [%u], "  \
                      "Dropping Request", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_NOT_SUFFICIENT_SIGNAL_ACQ_PERMISSION:
            vsnprintf(buffer, sizeof(buffer),
                      "Permission Check Failed for Signal [%u], "  \
                      "Client does not have sufficient Permissions to provision Signal", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_TARGET_CHECK_FAILED:
             vsnprintf(buffer, sizeof(buffer),
                       "Specified Signal [%u] is not enabled for provisioning on this Target", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_LOGICAL_TO_PHYSICAL_MAPPING_FAILED:
            vsnprintf(buffer, sizeof(buffer),
                      "Logical to Physical Core / Cluster Mapping for the "  \
                      "Resource [%u], Failed. Dropping Request", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_REQUEST_VALIDATED:
            vsnprintf(buffer, sizeof(buffer),
                      "Request with handle: %lld, Successfully Validated.", args);

            Logger::log(INFO, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::VERIFIER_STATUS_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Verification Failed for Request [%lld], Dropping Request.", args);

            Logger::log(ERROR, "RTN_REQUEST_VERIFIER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::RATE_LIMITER_RATE_LIMITED:
            vsnprintf(buffer, sizeof(buffer),
                      "Client TID: [%d] Rate Limited, Dropping Request [%lld].", args);

            Logger::log(INFO, "RTN_RATE_LIMITER", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::YAML_PARSE_ERROR:
            vsnprintf(buffer, sizeof(buffer),
                      "Failed to parse file: %s, Error: %s", args);

            Logger::log(ERROR, "RTN_YAML_PARSER", funcName, std::string(buffer));
            break;

        default:
            break;
    }

    va_end(args);
}
