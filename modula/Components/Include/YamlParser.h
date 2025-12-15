// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef YAML_PARSER_H
#define YAML_PARSER_H

#include <yaml.h>

#include "ErrCodes.h"

#define SETUP_LIBYAML_PARSING(filePath)                     \
    FILE* configFile = fopen(filePath.c_str(), "r");        \
    if(configFile == nullptr) {                             \
        return RC_FILE_NOT_FOUND;                           \
    }                                                       \
    yaml_parser_t parser;                                   \
    yaml_event_t event;                                     \
    if(!yaml_parser_initialize(&parser)) {                  \
        fclose(configFile);                                 \
        return RC_YAML_PARSING_ERROR;                       \
    }                                                       \
    yaml_parser_set_input_file(&parser, configFile);        \

#define TEARDOWN_LIBYAML_PARSING                            \
    yaml_parser_delete(&parser);                            \
    fclose(configFile);                                     \

#endif
