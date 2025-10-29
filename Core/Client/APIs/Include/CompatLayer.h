#ifndef COMPAT_LAYER_H
#define COMPAT_LAYER_H

#include "ResourceTunerAPIs.h"

#define PROP_VAL_LENGTH 92
typedef struct {
    char value[PROP_VAL_LENGTH];
} PropVal;

int32_t perf_lock_acq(int32_t, int32_t, int32_t[], int32_t);
int32_t perf_lock_rel(int32_t);
int32_t perf_lock_rel(int32_t handle);
PropVal perf_get_prop(const char* prop , const char* defVal);
int32_t perf_lock_acq_rel(int32_t handle, int32_t duration, int32_t list[], int32_t numArgs, int32_t reserveNumArgs);

#endif
