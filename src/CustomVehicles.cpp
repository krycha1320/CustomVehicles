#include <iostream>
#include <cstdarg>
#include <cstdio>

typedef unsigned int cell;
typedef void* AMX;

#define PLUGIN_DATA_LOGPRINTF 0
#define PLUGIN_DATA_AMX_EXPORTS 1
#define AMX_ERR_NONE 0

#ifndef AMX_NATIVE_CALL
#define AMX_NATIVE_CALL
#endif

// =====================================================================
// Global pointers
// =====================================================================
void (*logprintf)(const char*, ...) = nullptr;
void* pAMXFunctions = nullptr;

// =====================================================================
// Helper: safe logging macro
// =====================================================================
#define LOGF(fmt, ...) do { if (logprintf) logprintf(fmt, ##__VA_ARGS__); else printf(fmt "\n", ##__VA_ARGS__); } while(0)

// =====================================================================
// Test native: AddVehicleModel
// =====================================================================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params)
{
    LOGF("[CustomVehicles] AddVehicleModel called! (amx=%p)", amx);
    if (!params) {
        LOGF("[CustomVehicles] WARNING: params == nullptr!");
        return 0;
    }

    LOGF("[CustomVehicles] baseid=%d, newid=%d, dff_ptr=%p, txd_ptr=%p",
         (int)params[1], (int)params[2], (void*)params[3], (void*)params[4]);
    return 1;
}

// =====================================================================
// API entrypoints
// =====================================================================
extern "C" __attribute__((visibility("default"))) unsigned int Supports()
{
    LOGF("[CustomVehicles] DEBUG → Supports() called.");
    return 1 | 2;
}

extern "C" __attribute__((visibility("default"))) bool Load(void** ppData)
{
    LOGF("[CustomVehicles] DEBUG → Load() start");

    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];

    LOGF("[CustomVehicles] >> Load() called - plugin initialized!");
    LOGF("[CustomVehicles] DEBUG → logprintf ptr: %p", (void*)logprintf);
    LOGF("[CustomVehicles] DEBUG → pAMXFunctions ptr: %p", (void*)pAMXFunctions);

    // =================================================================
    // Attempt early registration
    // =================================================================
    typedef int (*AMXREGISTER)(AMX*, const void*, int);
    AMXREGISTER amx_Register_func = nullptr;

    if (pAMXFunctions)
    {
        amx_Register_func = (AMXREGISTER)((void**)pAMXFunctions)[12];
        LOGF("[CustomVehicles] DEBUG → amx_Register_func: %p", (void*)amx_Register_func);
    }
    else
    {
        LOGF("[CustomVehicles] ERROR → pAMXFunctions is nullptr!");
    }

    if (amx_Register_func)
    {
        static const struct {
            const char* name;
            cell (*func)(AMX*, cell*);
        } natives[] = {
            {"AddVehicleModel", n_AddVehicleModel},
            {nullptr, nullptr}
        };

        LOGF("[CustomVehicles] DEBUG → Attempting early registration...");
        int result = amx_Register_func(nullptr, natives, -1);
        LOGF("[CustomVehicles] DEBUG → amx_Register_func(nullptr) returned %d", result);
        LOGF("[CustomVehicles] Native AddVehicleModel registered (early).");
    }
    else
    {
        LOGF("[CustomVehicles] WARNING → Cannot obtain amx_Register_func!");
    }

    LOGF("[CustomVehicles] DEBUG → Load() end");
    return true;
}

// =====================================================================
// AMX Load — called when Pawn script attaches
// =====================================================================
extern "C" __attribute__((visibility("default"))) int AmxLoad(AMX* amx)
{
    LOGF("[CustomVehicles] DEBUG → AmxLoad() called (amx=%p)", amx);

    if (!pAMXFunctions) {
        LOGF("[CustomVehicles] ERROR → pAMXFunctions is NULL in AmxLoad!");
        return AMX_ERR_NONE;
    }

    typedef int (*AMXREGISTER)(AMX*, const void*, int);
    AMXREGISTER amx_Register_func = (AMXREGISTER)((void**)pAMXFunctions)[12];
    LOGF("[CustomVehicles] DEBUG → amx_Register_func: %p", (void*)amx_Register_func);

    if (!amx_Register_func) {
        LOGF("[CustomVehicles] ERROR → amx_Register_func pointer is NULL!");
        return AMX_ERR_NONE;
    }

    static const struct {
        const char* name;
        cell (*func)(AMX*, cell*);
    } natives[] = {
        {"AddVehicleModel", n_AddVehicleModel},
        {nullptr, nullptr}
    };

    int res = amx_Register_func(amx, natives, -1);
    LOGF("[CustomVehicles] DEBUG → amx_Register_func(amx) returned %d", res);
    LOGF("[CustomVehicles] Registered Pawn native: AddVehicleModel");
    return AMX_ERR_NONE;
}

// =====================================================================
// AMX Unload — when script detaches
// =====================================================================
extern "C" __attribute__((visibility("default"))) int AmxUnload(AMX* amx)
{
    LOGF("[CustomVehicles] DEBUG → AmxUnload() called (amx=%p)", amx);
    return AMX_ERR_NONE;
}
