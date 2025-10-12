#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <sys/stat.h>

typedef unsigned int cell;
typedef void* AMX;

#define PLUGIN_DATA_LOGPRINTF 0
#define AMX_ERR_NONE 0

#ifndef AMX_NATIVE_CALL
#define AMX_NATIVE_CALL
#endif

// ===================================================
// Globalne wskaźniki
// ===================================================
void (*logprintf)(const char*, ...) = nullptr;

// ===================================================
// Struktura pojazdu
// ===================================================
struct VehicleDef {
    int baseid;
    int newid;
    std::string dff;
    std::string txd;
};

std::vector<VehicleDef> g_Vehicles;

// ===================================================
// Logger awaryjny
// ===================================================
void DefaultLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

// ===================================================
// Zapis pojazdów do prostego pliku tekstowego
// ===================================================
void SaveVehicles()
{
#ifdef _WIN32
    _mkdir("scriptfiles");
#else
    mkdir("scriptfiles", 0777);
#endif

    std::ofstream file("scriptfiles/vehicles.json");
    if (!file.is_open())
    {
        logprintf("[CustomVehicles] ERROR: cannot write vehicles.json");
        return;
    }

    for (const auto& v : g_Vehicles)
    {
        file << v.baseid << " " << v.newid << " "
             << v.dff << " " << v.txd << "\n";
    }
    file.close();
    logprintf("[CustomVehicles] Saved %zu vehicles (plain text).", g_Vehicles.size());
}

// ===================================================
// Wczytanie pojazdów z pliku tekstowego
// ===================================================
void LoadVehicles()
{
    std::ifstream file("scriptfiles/vehicles.json");
    if (!file.is_open())
    {
        logprintf("[CustomVehicles] No vehicles.json found, skipping load.");
        return;
    }

    g_Vehicles.clear();

    int baseid, newid;
    std::string dff, txd;
    while (file >> baseid >> newid >> dff >> txd)
    {
        g_Vehicles.push_back({baseid, newid, dff, txd});
        logprintf("[CustomVehicles] Loaded model base %d → new %d (%s / %s)",
                  baseid, newid, dff.c_str(), txd.c_str());
    }

    logprintf("[CustomVehicles] Successfully loaded %zu vehicles (plain text).", g_Vehicles.size());
    file.close();
}

// ===================================================
// Native AddVehicleModel
// ===================================================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params)
{
    int baseid = (int)params[1];
    int newid  = (int)params[2];
    const char* dff = reinterpret_cast<const char*>(params[3]);
    const char* txd = reinterpret_cast<const char*>(params[4]);

    logprintf("[CustomVehicles] AddVehicleModel called! baseid=%d, newid=%d, dff=%s, txd=%s",
              baseid, newid, dff, txd);

    g_Vehicles.push_back({baseid, newid, dff, txd});
    SaveVehicles();
    return 1;
}

// ===================================================
// Plugin entrypoints
// ===================================================
extern "C" __attribute__((visibility("default"))) unsigned int Supports()
{
    return 1 | 2;
}

extern "C" __attribute__((visibility("default"))) bool Load(void** ppData)
{
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    if (!logprintf) logprintf = DefaultLog;

    logprintf(">> [CustomVehicles] Loaded (plain-text mode)");
    LoadVehicles();
    return true;
}

extern "C" __attribute__((visibility("default"))) int AmxLoad(AMX* amx)
{
    logprintf("[CustomVehicles] AmxLoad() called (compatibility mode)");
    return AMX_ERR_NONE;
}

extern "C" __attribute__((visibility("default"))) int AmxUnload(AMX* amx)
{
    logprintf("[CustomVehicles] Unload()");
    return AMX_ERR_NONE;
}
