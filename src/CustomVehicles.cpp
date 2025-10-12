#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdarg>
#include <sys/stat.h>
#include <cstring>

// ===============================================
// Minimalne definicje SA-MP / open.mp API
// ===============================================
typedef unsigned int cell;
typedef void* AMX;

#define AMX_ERR_NONE 0
#define PLUGIN_DATA_LOGPRINTF 0
#define PLUGIN_DATA_AMX_EXPORTS 1

#ifndef AMX_NATIVE_CALL
    #define AMX_NATIVE_CALL
#endif

void (*logprintf)(const char*, ...) = nullptr;
void *pAMXFunctions = nullptr;

// ===============================================
// Pomocniczy logger awaryjny
// ===============================================
void DefaultLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

// ===============================================
// Dane pojazdów
// ===============================================
struct VehicleDef {
    int baseid;
    int newid;
    std::string dff;
    std::string txd;
};

std::vector<VehicleDef> g_Vehicles;

// ===============================================
// Zapis pojazdów do JSON
// ===============================================
void SaveVehiclesJSON()
{
#ifdef _WIN32
    _mkdir("scriptfiles");
#else
    mkdir("scriptfiles", 0777);
#endif

    std::ofstream file("scriptfiles/vehicles.json");
    if (!file.is_open()) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: cannot open vehicles.json for writing!");
        return;
    }

    file << "[\n";
    for (size_t i = 0; i < g_Vehicles.size(); ++i) {
        const auto& v = g_Vehicles[i];
        file << "  {\n";
        file << "    \"baseid\": " << v.baseid << ",\n";
        file << "    \"newid\": " << v.newid << ",\n";
        file << "    \"dff\": \"" << v.dff << "\",\n";
        file << "    \"txd\": \"" << v.txd << "\"\n";
        file << "  }";
        if (i < g_Vehicles.size() - 1) file << ",";
        file << "\n";
    }
    file << "]";
    file.close();

    if (logprintf)
        logprintf("[CustomVehicles] Saved %zu vehicles to scriptfiles/vehicles.json", g_Vehicles.size());
}

// ===============================================
// Dodawanie pojazdu (zapis + log)
// ===============================================
cell AddVehicleModel_Internal(int baseid, int newid, const char* dff, const char* txd)
{
    if (!dff || !txd) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: Missing DFF or TXD name!");
        return 0;
    }

    if (newid < 20000) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: Invalid new ID (%d)", newid);
        return 0;
    }

    VehicleDef v { baseid, newid, dff, txd };
    g_Vehicles.push_back(v);
    SaveVehiclesJSON();

    if (logprintf)
        logprintf("[CustomVehicles] Added model: base %d → new %d (%s / %s)",
                  baseid, newid, dff, txd);
    return 1;
}

// ===============================================
// Pomocnicze pobranie stringa z AMX
// ===============================================
static void amx_GetString(char *dest, cell addr, int /*use_wchar*/, size_t size)
{
    if (!addr || !dest || size == 0) return;
    const char *src = (const char*)addr;
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

// ===============================================
// Pawn native: AddVehicleModel(baseid, newid, dff[], txd[])
// ===============================================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params)
{
    int baseid = (int)params[1];
    int newid  = (int)params[2];

    char dff[64], txd[64];
    amx_GetString(dff, params[3], 0, sizeof(dff));
    amx_GetString(txd, params[4], 0, sizeof(txd));

    return AddVehicleModel_Internal(baseid, newid, dff, txd);
}

// ===============================================
// Funkcje wymagane przez open.mp plugin API
// ===============================================
extern "C" __attribute__((visibility("default"))) unsigned int Supports()
{
    return 1 | 2;
}

extern "C" __attribute__((visibility("default"))) bool Load(void **ppData)
{
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];

    if (!logprintf) logprintf = DefaultLog;
    logprintf(">> [CustomVehicles] Load() called - plugin initialized!");
    return true;
}

extern "C" __attribute__((visibility("default"))) void Unload()
{
    if (logprintf) logprintf(">> [CustomVehicles] Unload() called!");
}

extern "C" __attribute__((visibility("default"))) int AmxLoad(AMX *amx)
{
    if (logprintf) logprintf("[CustomVehicles] AmxLoad() called - registering natives...");

    typedef int (*AMXREGISTER)(AMX*, const void*, int);
    AMXREGISTER amx_Register_func = (AMXREGISTER)((void**)pAMXFunctions)[12];

    static const struct {
        const char *name;
        cell (*func)(AMX*, cell*);
    } natives[] = {
        {"AddVehicleModel", n_AddVehicleModel},
        {nullptr, nullptr}
    };

    amx_Register_func(amx, natives, -1);
    if (logprintf) logprintf("[CustomVehicles] Registered Pawn native: AddVehicleModel");
    return AMX_ERR_NONE;
}

extern "C" __attribute__((visibility("default"))) int AmxUnload(AMX *amx)
{
    if (logprintf) logprintf("[CustomVehicles] AmxUnload() called!");
    return AMX_ERR_NONE;
}
