#include "plugin.h"
#include <string>
#include <vector>
#include <fstream>

// =====================================================
//  Plugin information
// =====================================================

typedef void (*logprintf_t)(const char* format, ...);
static logprintf_t logprintf;

// =====================================================
//  Vehicle struct
// =====================================================

struct VehicleDef {
    int baseid;
    int newid;
    std::string dff;
    std::string txd;
};

static std::vector<VehicleDef> g_Vehicles;

// =====================================================
//  Native: AddVehicleModel(baseid, newid, dff, txd)
// =====================================================

cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params)
{
    int baseid = (int)params[1];
    int newid = (int)params[2];

    char* dff; char* txd;
    amx_StrParam(amx, params[3], dff);
    amx_StrParam(amx, params[4], txd);

    if (!dff || !txd) return 0;

    VehicleDef v = { baseid, newid, dff, txd };
    g_Vehicles.push_back(v);

    logprintf("[CustomVehicles] Registered vehicle ID %d (base %d) with %s/%s",
        newid, baseid, dff, txd);

    return 1;
}

// =====================================================
//  Save vehicles.json
// =====================================================

static void SaveVehiclesJSON()
{
    std::ofstream file("scriptfiles/vehicles.json");
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
    file << "]";
    file.close();

    logprintf("[CustomVehicles] vehicles.json saved (%zu entries)", g_Vehicles.size());
}

// =====================================================
//  Plugin Core
// =====================================================

extern void* pAMXFunctions;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

    logprintf(">> CustomVehicles plugin loaded! (OMP-compatible)");

    return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
    SaveVehiclesJSON();
    logprintf(">> CustomVehicles plugin unloaded!");
}

AMX_NATIVE_INFO natives[] =
{
    { "AddVehicleModel", n_AddVehicleModel },
    { NULL, NULL }
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
    logprintf(">> CustomVehicles: registering natives...");
    return amx_Register(amx, natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
    logprintf(">> CustomVehicles: AMX unloaded.");
    return AMX_ERR_NONE;
}
