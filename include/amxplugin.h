// ============================================================
// amxplugin.h — poprawiona wersja Zeex SDK + Linux/open.mp fix
// ============================================================

#pragma once

#include "amx/amx.h"
#include "plugincommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AMXPLUGIN_H_INCLUDED
#define AMXPLUGIN_H_INCLUDED

// -------------------------------------------------------------
// Plugin data indexes
// -------------------------------------------------------------
#define PLUGIN_DATA_LOGPRINTF          0x00
#define PLUGIN_DATA_AMX_EXPORTS        0x10
#define PLUGIN_DATA_CALLPUBLIC_FS      0x20
#define PLUGIN_DATA_CALLPUBLIC_GM      0x21
#define PLUGIN_DATA_NATIVE             0x30
#define PLUGIN_DATA_CALLBACK           0x40

// -------------------------------------------------------------
// Typedefs
// -------------------------------------------------------------
typedef void (*logprintf_t)(const char* format, ...);

typedef int   (AMXAPI *amx_Register_t)(AMX *amx, const AMX_NATIVE_INFO *nativelist, int number);
typedef int   (AMXAPI *amx_FindPublic_t)(AMX *amx, const char *name, int *index);
typedef int   (AMXAPI *amx_Exec_t)(AMX *amx, cell *retval, int index);
typedef int   (AMXAPI *amx_GetAddr_t)(AMX *amx, cell amx_addr, cell **phys_addr);
typedef int   (AMXAPI *amx_Allot_t)(AMX *amx, int cells, cell *amx_addr, cell **phys_addr);
typedef int   (AMXAPI *amx_Release_t)(AMX *amx, cell amx_addr);
typedef int   (AMXAPI *amx_StrLen_t)(const cell *cstring, int *length);
typedef int   (AMXAPI *amx_GetString_t)(char *dest, const cell *source, int use_wchar, size_t size);
typedef int   (AMXAPI *amx_SetString_t)(cell *dest, const char *source, int pack, int use_wchar, size_t size);

// -------------------------------------------------------------
// AMX function helpers (fix for missing typedefs in some SDKs)
// -------------------------------------------------------------
#ifndef AMX_NATIVE
typedef int (AMX_NATIVE_CALL *AMX_NATIVE)(AMX *amx, cell *params);
#endif

#ifndef AMX_CALLBACK
typedef int (AMXAPI *AMX_CALLBACK)(AMX *amx, cell index, cell *result, cell *params);
#endif

#ifndef AMX_DEBUG
typedef int (AMXAPI *AMX_DEBUG)(AMX *amx);
#endif

// -------------------------------------------------------------
// Export macros
// -------------------------------------------------------------
#ifndef PLUGIN_EXPORT
#define PLUGIN_EXPORT extern "C"
#endif

#ifndef PLUGIN_CALL
#define PLUGIN_CALL
#endif

// -------------------------------------------------------------
// Native info helper
// -------------------------------------------------------------
typedef AMX_NATIVE_INFO* (AMXAPI *amx_NativeInfo_t)(const char* name, AMX_NATIVE func);

static inline AMX_NATIVE_INFO* AMXAPI amx_NativeInfo(const char* name, AMX_NATIVE func)
{
    static AMX_NATIVE_INFO native;
    native.name = name;
    native.func = func;
    return &native;
}

// -------------------------------------------------------------
// Callback helpers
// -------------------------------------------------------------
typedef int  (AMXAPI *amx_SetCallback_t)(AMX* amx, AMX_CALLBACK callback);
static inline int AMXAPI amx_SetCallback(AMX* amx, AMX_CALLBACK callback)
{
    if (!amx) return AMX_ERR_NONE;
    amx->callback = callback;
    return AMX_ERR_NONE;
}

typedef int  (AMXAPI *amx_SetDebugHook_t)(AMX* amx, AMX_DEBUG debug);
static inline int AMXAPI amx_SetDebugHook(AMX* amx, AMX_DEBUG debug)
{
    if (!amx) return AMX_ERR_NONE;
    amx->debug = debug;
    return AMX_ERR_NONE;
}

// -------------------------------------------------------------
// Utility macros
// -------------------------------------------------------------
#define CHECK_PARAMS(n) \
    if (params[0] != (n * sizeof(cell))) { \
        if (logprintf) logprintf("[CustomVehicles] Invalid parameter count for native."); \
        return 0; \
    }

#define GET_STRING(amx, param, result) \
    do { \
        cell *amx_cstr; \
        int amx_length; \
        amx_GetAddr((amx), (param), &amx_cstr); \
        amx_StrLen(amx_cstr, &amx_length); \
        if (amx_length > 0 && ((result) = (char*)alloca(amx_length + 1))) \
            amx_GetString((result), amx_cstr, 0, amx_length + 1); \
        else (result) = NULL; \
    } while (0)

#endif /* AMXPLUGIN_H_INCLUDED */

#ifdef __cplusplus
}
#endif
