// ============================================================
// amxplugin.h — SA:MP / open.mp Plugin SDK header
// Poprawiona wersja kompatybilna z open.mp i Linux (32-bit)
// ============================================================

#pragma once

#include "amx/amx.h"
#include "plugincommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AMXPLUGIN_H_INCLUDED
#define AMXPLUGIN_H_INCLUDED

/* -----------------------------------------------------------------
   Typy danych
   ----------------------------------------------------------------- */

typedef void (*logprintf_t)(const char* format, ...);

/* -----------------------------------------------------------------
   Dane pluginu
   ----------------------------------------------------------------- */

#define PLUGIN_DATA_LOGPRINTF          0x00
#define PLUGIN_DATA_AMX_EXPORTS        0x10
#define PLUGIN_DATA_CALLPUBLIC_FS      0x20
#define PLUGIN_DATA_CALLPUBLIC_GM      0x21
#define PLUGIN_DATA_NATIVE             0x30
#define PLUGIN_DATA_CALLBACK           0x40

/* -----------------------------------------------------------------
   Exportowane funkcje AMX
   ----------------------------------------------------------------- */

typedef int   (AMXAPI *amx_Register_t)(AMX *amx, const AMX_NATIVE_INFO *nativelist, int number);
typedef int   (AMXAPI *amx_FindPublic_t)(AMX *amx, const char *name, int *index);
typedef int   (AMXAPI *amx_Exec_t)(AMX *amx, cell *retval, int index);
typedef int   (AMXAPI *amx_GetAddr_t)(AMX *amx, cell amx_addr, cell **phys_addr);
typedef int   (AMXAPI *amx_Allot_t)(AMX *amx, int cells, cell *amx_addr, cell **phys_addr);
typedef int   (AMXAPI *amx_Release_t)(AMX *amx, cell amx_addr);
typedef int   (AMXAPI *amx_StrLen_t)(const cell *cstring, int *length);
typedef int   (AMXAPI *amx_GetString_t)(char *dest, const cell *source, int use_wchar, size_t size);
typedef int   (AMXAPI *amx_SetString_t)(cell *dest, const char *source, int pack, int use_wchar, size_t size);

/* -----------------------------------------------------------------
   Typy funkcji używane przez pluginy
   ----------------------------------------------------------------- */

typedef void (*PLUGIN_LOAD)();
typedef void (*PLUGIN_UNLOAD)();

/* -----------------------------------------------------------------
   Natywne makra pomocnicze
   ----------------------------------------------------------------- */

#ifndef AMX_NATIVE_CALL
#define AMX_NATIVE_CALL
#endif

#ifndef AMXAPI
#define AMXAPI
#endif

#ifndef PLUGIN_CALL
#define PLUGIN_CALL
#endif

#ifndef PLUGIN_EXPORT
#define PLUGIN_EXPORT extern "C"
#endif

/* -----------------------------------------------------------------
   Rejestracja natywek
   ----------------------------------------------------------------- */

typedef AMX_NATIVE_INFO* AMXAPI (*amx_NativeInfo_t)(const char* name, AMX_NATIVE func);

static inline AMX_NATIVE_INFO* AMXAPI amx_NativeInfo(const char* name, AMX_NATIVE func)
{
    static AMX_NATIVE_INFO native;
    native.name = name;
    native.func = func;
    return &native;
}

/* -----------------------------------------------------------------
   Obsługa callbacków i debuggera
   ----------------------------------------------------------------- */

typedef int  AMXAPI (*amx_SetCallback_t)(AMX* amx, AMX_CALLBACK callback);
static inline int AMXAPI amx_SetCallback(AMX* amx, AMX_CALLBACK callback)
{
    if (!amx) return AMX_ERR_NONE;
    amx->callback = callback;
    return AMX_ERR_NONE;
}

typedef int  AMXAPI (*amx_SetDebugHook_t)(AMX* amx, AMX_DEBUG debug);
static inline int AMXAPI amx_SetDebugHook(AMX* amx, AMX_DEBUG debug)
{
    if (!amx) return AMX_ERR_NONE;
    amx->debug = debug;
    return AMX_ERR_NONE;
}

/* -----------------------------------------------------------------
   Pomocnicze makra do AMX
   ----------------------------------------------------------------- */

#define CHECK_PARAMS(n) \
    if (params[0] != (n * sizeof(cell))) { \
        if (logprintf) logprintf("[CustomVehicles] Bad parameter count in native!"); \
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
