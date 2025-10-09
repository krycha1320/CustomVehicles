// plugin.h - SA:MP Plugin SDK header (simplified)
#pragma once

#include "amx/amx.h"
#include "plugincommon.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef void (*logprintf_t)(const char* format, ...);

#define PLUGIN_EXPORT extern "C"
#define PLUGIN_CALL
#define PLUGIN_DATA_LOGPRINTF   0x00
#define PLUGIN_DATA_AMX_EXPORTS 0x10
#define PLUGIN_DATA_CALLPUBLIC_FS 0x20
#define PLUGIN_DATA_CALLPUBLIC_GM 0x21

typedef unsigned int (*Supports_t)();
typedef bool (*Load_t)(void** ppData);
typedef void (*Unload_t)();

#ifdef __cplusplus
  }
#endif
