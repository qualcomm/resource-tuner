// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Logger.h"

int8_t Logger::mCurrentLevel = ERROR;
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

    // Add a property, to log to file OR Ftrace
    // Default: log to ftrace
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

            Logger::log(ERROR, "URM_CLIENT_DATA_MANAGER",
                        funcName, std::string(buffer));
            break;

        case CommonMessageTypes::CLIENT_ALLOCATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                     "Memory allocation for Client: " \
                     "[PID: %d, TID: %d]. Failed with error: %s", args);

            Logger::log(ERROR, "URM_CLIENT_DATA_MANAGER",
                        funcName, std::string(buffer));
            break;

        case CommonMessageTypes::PROPERTY_RETRIEVAL_FAILED:
            Logger::log(ERROR, "URM_SYSTUNE_MAIN", funcName,
                        "Failed to Fetch Properties, " \
                        "Boot Configs, Systune Server Initialization Failed.");
            break;

        case CommonMessageTypes::META_CONFIG_PARSE_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                     "Fetch Meta Configs failed, with error %s", args);

            Logger::log(ERROR, "URM_SYSTUNE_MAIN", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_INIT_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Could Not Initialize Thread Pool. Error: %s", args);

            Logger::log(ERROR, "URM_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_THREAD_ALLOCATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Failed to allocate Memory for ThreadNode. " \
                      "Error: %s", args);

            Logger::log(ERROR, "URM_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_THREAD_CREATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Failed to create New thread, call to std::thread Failed. " \
                      "Error: %s", args);

            Logger::log(ERROR, "URM_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_THREAD_TERMINATED:
            vsnprintf(buffer, sizeof(buffer),
                      "Thread Terminated with error: %s", args);

            Logger::log(ERROR, "URM_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_ENQUEUE_TASK_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Task Submisission failed with error: %s", args);

            Logger::log(ERROR, "URM_THREAD_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::THREAD_POOL_FULL_ALERT:
            LOGE("URM_THREAD_POOL",
                 "ThreadPool is full, Task Submission Failed");

        case CommonMessageTypes::PULSE_MONITOR_INIT_FAILED:
            Logger::log(ERROR, "URM_SYSTUNE_MAIN", funcName,
                        "Pulse Monitor Could not be started, " \
                        "Aborting Initialization.");
            break;

        case CommonMessageTypes::GARBAGE_COLLECTOR_INIT_FAILED:
            Logger::log(ERROR, "URM_SYSTUNE_MAIN", funcName,
                        "Client Garbage Collector Could not be started, " \
                        "Aborting Initialization.");
            break;

        case CommonMessageTypes::MEMORY_POOL_INVALID_BLOCK_SIZE:
            vsnprintf(buffer, sizeof(buffer),
                     "Invalid block Size Provided [size = %d bytes.]", args);

            Logger::log(ERROR, "URM_MEMORY_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "No Free Blocks of Requested Size Available " \
                      "[size = %d bytes.]", args);

            Logger::log(ERROR, "URM_MEMORY_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::MEMORY_POOL_ALLOCATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Insufficient Memory to Allocate Desired Number of Blocks" \
                      "[Requested Size = %d bytes, "                             \
                      "Requested Count = %d, "                                   \
                      "Allocated Count = %d.]", args);

            Logger::log(ERROR, "URM_MEMORY_POOL", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::MODULE_INIT_FAILED:
            vsnprintf(buffer, sizeof(buffer),
                      "Failed to Initialize Module: %s, " \
                      "Aborting Server Startup.", args);

            Logger::log(ERROR, "URM_SYSTUNE_MAIN", funcName, std::string(buffer));
            break;

        case CommonMessageTypes::REQUEST_MEMORY_ALLOCATION_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Memory allocation for Incoming Request: " \
                      "[TYPE: %s], Failed with Error: %s", args);

            Logger::log(ERROR, "URM_REQUEST_ALLOCATION_FAILURE",
                        funcName, std::string(buffer));
            break;

        case CommonMessageTypes::REQUEST_PARSING_FAILURE:
            vsnprintf(buffer, sizeof(buffer),
                      "Request Parsing Failed, Request is Malformed: " \
                      "[TYPE: %s ]. Error:  %s", args);

            Logger::log(ERROR, "URM_REQUEST_PARSING_FAILURE",
                        funcName, std::string(buffer));
            break;

        case CommonMessageTypes::ERRNO_LOG:
            vsnprintf(buffer, sizeof(buffer),
                      "Call to %s, Failed with Error: %s", args);

            Logger::log(ERROR, "URM_SYSCALL_FAILURE", funcName, std::string(buffer));
            break;

        default:
            break;
    }

    va_end(args);
}
