// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <ctime>
#include <chrono>
#include <fstream>
#include <cstdarg>

#define LOGD(tag, message) Logger::log(DEBUG, tag, __func__, message)
#define LOGI(tag, message) Logger::log(INFO, tag, __func__, message)
#define LOGE(tag, message) Logger::log(ERROR, tag, __func__, message)
#define TYPELOGV(type, args...) Logger::typeLog(type, __func__, args)
#define TYPELOGD(type) Logger::typeLog(type, __func__)

enum LogLevel {
    DEBUG = 0,
    INFO = 1,
    ERROR = 2
};

enum RedirectOptions {
    FTRACE,
    LOG_FILE
};

enum CommonMessageTypes {
    ERRNO_LOG,
    CLIENT_ALLOCATION_FAILURE,
    PROPERTY_RETRIEVAL_FAILED,
    PULSE_MONITOR_INIT_FAILED,
    GARBAGE_COLLECTOR_INIT_FAILED,
    MEMORY_POOL_INVALID_BLOCK_SIZE,
    MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE,
    MEMORY_POOL_ALLOCATION_FAILURE,
    NOTIFY_MODULE_ENABLED,
    MODULE_INIT_FAILED,
    REQUEST_MEMORY_ALLOCATION_FAILURE,
    REQUEST_PARSING_FAILURE,
    META_CONFIG_PARSE_FAILURE,
    THREAD_POOL_INIT_FAILURE,
    THREAD_POOL_THREAD_ALLOCATION_FAILURE,
    THREAD_POOL_THREAD_CREATION_FAILURE,
    THREAD_POOL_THREAD_TERMINATED,
    THREAD_POOL_ENQUEUE_TASK_FAILURE,
    THREAD_POOL_FULL_ALERT
};

/**
* @brief Logger.
* @details Provides a Simplified and Consistent interface for Logging across different Targets
* Currently We support 3 levels of Logging
* 1. Debug - For almost all non-essential debug statements.
* 2. Info - For essential statements.
* 3. Error - Statements if printed, shows errors.
*/
class Logger {
private:
    static int8_t mCurrentLevel;
    static int8_t mLevelSpecificLogging;
    static RedirectOptions mRedirectOutputTo;

    static std::string getTimestamp();
    static std::string levelToString(LogLevel level);

public:
    /**
     * @brief Configure the Logger
     * @details The logger is designed to be Customizable. Among the knobs which can be configured are:
                1) What levels to Log, (User can specify an exact value say ERROR, or a Lower Bound say INFO).
                   In case of Lower Bound Config for Logging, any Message with Log Level higher than the
                   specified Level will be logged.
                2) Output Redirection: Output can be captured in a File or directly in ftrace
     * @param level The exact level to Log or the Lower Bound Logging level.
     * @param levelSpecificLogging Indicates whether Exact Level Logging is needed, default behaviour is
     *                             Lower Bound Logging.
     * @param redirectOutputTo Indicates whether a new file needs to be created to capture the Logging,
     *                         Or Ftrace should be used.
     * @return int32_t: Number of blocks which were actually allocated (<= blockCount)
     */
    static void configure(int8_t level, int8_t levelSpecificLogging, RedirectOptions redirectOutputTo);

    /**
     * @brief Responsible for actually Logging a Message to the desired Medium (file or Ftrace)
     * @details Note, this Routine should not be called directly, instead the Macros
     *          LOGD, LOGE, LOGI, TYPELOGD and TYPELOGV should be used.
     */
    static void log(LogLevel level, const std::string& tag, const std::string& funcName, const std::string& message);
    static void typeLog(CommonMessageTypes type, const std::string& funcName, ...);
};

#endif
