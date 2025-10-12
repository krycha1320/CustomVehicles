#include <iostream>
#include <cstdarg>

typedef unsigned int cell;
typedef void* AMX;

#define PLUGIN_DATA_LOGPRINTF 0
#define PLUGIN_DATA_AMX_EXPORTS 1
#define AMX_ERR_NONE 0

#ifndef AMX_NATIVE_CALL
#define AMX_NATIVE_CALL
#endif

void (*logprintf)(const char*, ...) = nullptr;
void* pAMXFunctions = nullptr;

cell AMX_NATIVE_CALL n_TestNative(AMX* amx, cell* params)
{
    if (logprintf) logprintf("[CustomVehicles] Native AddVehicleModel called!");
    return 1;
}

extern "C" __attribute__((visibility("default"))) unsigned int Supports()
{
    return 1 | 2;
}

extern "C" __attribute__((visibility("default"))) bool Load(void** ppData)
{
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    if (logprintf) logprintf(">> [CustomVehicles] Load() called - plugin initialized!");
    return true;
}

extern "C" __attribute__((visibility("default"))) int AmxLoad(AMX* amx)
{
    if (logprintf) logprintf("[CustomVehicles] AmxLoad() called - registering natives...");
    typedef int (*AMXREGISTER)(AMX*, const void*, int);
    AMXREGISTER amx_Register_func = (AMXREGISTER)((void**)pAMXFunctions)[12];

    static const struct {
        const char* name;
        cell (*func)(AMX*, cell*);
    } natives[] = {
        {"AddVehicleModel", n_TestNative},
        {nullptr, nullptr}
    };

    amx_Register_func(amx, natives, -1);
    if (logprintf) logprintf("[CustomVehicles] Registered Pawn native: AddVehicleModel");
    return AMX_ERR_NONE;
}
