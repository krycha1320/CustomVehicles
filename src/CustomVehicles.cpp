#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <sys/stat.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
// Pomocnicze funkcje do JSON
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

    json j = json::array();
    for (const auto& v : g_Vehicles)
    {
        j.push_back({
            {"baseid", v.baseid},
            {"newid", v.newid},
            {"dff", v.dff},
            {"txd", v.txd}
        });
    }

    file << j.dump(4);
    file.close();

    logprintf("[CustomVehicles] Saved %zu vehicles to scriptfiles/vehicles.json", g_Vehicles.size());
}

void LoadVehicles()
{
    std::ifstream file("scriptfiles/vehicles.json");
    if (!file.is_open())
    {
        logprintf("[CustomVehicles] No vehicles.json found, skipping load.");
        return;
    }

    try
    {
        json j;
        file >> j;
        file.close();

        g_Vehicles.clear();

        for (const auto& v : j)
        {
            VehicleDef def;
            def.baseid = v.value("baseid", 0);
            def.newid = v.value("newid", 0);
            def.dff = v.value("dff", "");
            def.txd = v.value("txd", "");
            g_Vehicles.push_back(def);

            logprintf("[CustomVehicles] Loaded model base %d → new %d (%s / %s)",
                def.baseid, def.newid, def.dff.c_str(), def.txd.c_str());
        }

        logprintf("[CustomVehicles] Successfully loaded %zu vehicles from JSON.", g_Vehicles.size());
    }
    catch (std::exception& e)
    {
        logprintf("[CustomVehicles] ERROR parsing vehicles.json: %s", e.what());
    }
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

    VehicleDef v{baseid, newid, dff, txd};
    g_Vehicles.push_back(v);
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

    logprintf(">> [CustomVehicles] Loaded (legacy-safe mode with JSON support)");
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
