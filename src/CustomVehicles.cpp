// ================================================================
// CustomVehicles plugin for SA:MP / open.mp
// Author: Krycha1320 (Desert Project)
// ================================================================

// najpierw AMX, potem plugincommon i amxplugin
#include "amx/amx.h"
#include "plugincommon.h"
#include "amxplugin.h"
#include "plugin.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

// Typ do logowania z serwera
typedef void (*logprintf_t)(const char* format, ...);
static logprintf_t logprintf = nullptr;

// Struktura pojazdu
struct VehicleDef {
    int baseid;
    int newid;
    std::string dff;
    std::string txd;
};

// Wektor na nowe pojazdy
static std::vector<VehicleDef> g_Vehicles;

// =====================================================
// NATYWKA AddVehicleModel
// =====================================================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params)
{
    int baseid = (int)params[1];
    int newid = (int)params[2];

    char* dff;
    char* txd;
    amx_StrParam(amx, params[3], dff);
    amx_StrParam(amx, params[4], txd);

    if (!dff || !txd) return 0;

    VehicleDef v = { baseid, newid, dff, txd };
    g_Vehicles.push_back(v);

    if (logprintf)
        logprintf("[CustomVehicles] Registered vehicle ID %d (base %d) with %s / %s",
            newid, baseid, dff, txd);

    return 1;
}

// =====================================================
// ZAPIS JSON (scriptfiles/vehicles.json)
// =====================================================
static void SaveVehiclesJSON()
{
    std::ofstream file("scriptfiles/vehicles.json");
    if (!file.is_open()) {
        if (logprintf)
            logprintf("[CustomVehicles] ERROR: Cannot open scriptfiles/vehicles.json for writing!");
        return;
    }

    file << "[\n";
    for (size_t i = 0; i < g_Vehicles.size(); i++) {
        const auto& v = g_Vehicles[i];
        file << "  { \"newid\": " << v.newid
             << ", \"baseid\": " << v.baseid
             << ", \"dff\": \"" << v.dff
             << "\", \"txd\": \"" << v.txd << "\" }";
        if (i + 1 < g_Vehicles.size()) file << ",";
        file << "\n";
    }
    file << "]\n";
    file.close();

    if (logprintf)
        logprintf("[CustomVehicles] vehicles.json saved with %d entries", (int)g_Vehicles.size());
}

// =====================================================
// FUNKCJE PLUGINU
// =====================================================
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
    logprintf(">> CustomVehicles plugin loaded!");
    return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
    SaveVehiclesJSON();
    if (logprintf)
        logprintf(">> CustomVehicles plugin unloaded!");
}

// =====================================================
// Rejestracja natywek
// =====================================================
static AMX_NATIVE_INFO custom_veh_Natives[] =
{
    { "AddVehicleModel", n_AddVehicleModel },
    { 0, 0 }
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
    return amx_Register(amx, custom_veh_Natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
    return AMX_ERR_NONE;
}
