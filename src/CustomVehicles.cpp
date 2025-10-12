#include <omp/Plugin.h>
#include <omp/Callback.h>
#include <omp/Log.h>
#include <omp/AMX.h>
#include <vector>
#include <string>
#include <fstream>
#include <sys/stat.h>

using namespace ompp;

struct VehicleDef {
    int baseid;
    int newid;
    std::string dff;
    std::string txd;
};

std::vector<VehicleDef> g_Vehicles;

void SaveVehicles()
{
#ifdef _WIN32
    _mkdir("scriptfiles");
#else
    mkdir("scriptfiles", 0777);
#endif

    std::ofstream file("scriptfiles/vehicles.txt");
    if (!file.is_open()) {
        Log::Error("[CustomVehicles] Cannot open vehicles.txt for writing!");
        return;
    }

    for (const auto& v : g_Vehicles)
        file << v.baseid << " " << v.newid << " " << v.dff << " " << v.txd << "\n";

    Log::Info("[CustomVehicles] Saved {} vehicles.", g_Vehicles.size());
}

void LoadVehicles()
{
    std::ifstream file("scriptfiles/vehicles.txt");
    if (!file.is_open()) {
        Log::Info("[CustomVehicles] No vehicles.txt found, skipping load.");
        return;
    }

    g_Vehicles.clear();
    int baseid, newid;
    std::string dff, txd;
    while (file >> baseid >> newid >> dff >> txd)
        g_Vehicles.push_back({baseid, newid, dff, txd});

    Log::Info("[CustomVehicles] Loaded {} vehicles from file.", g_Vehicles.size());
}

cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params)
{
    int baseid = static_cast<int>(params[1]);
    int newid = static_cast<int>(params[2]);
    const char* dff = reinterpret_cast<const char*>(params[3]);
    const char* txd = reinterpret_cast<const char*>(params[4]);

    g_Vehicles.push_back({baseid, newid, dff, txd});
    Log::Info("[CustomVehicles] AddVehicleModel called ({} -> {}, {} / {})",
              baseid, newid, dff, txd);
    SaveVehicles();
    return 1;
}

class CustomVehicles : public Plugin
{
public:
    bool onLoad(PluginData& data) override
    {
        Log::Info(">> [CustomVehicles] Loaded (open.mp native API)");
        LoadVehicles();

        // Rejestracja funkcji widocznej w Pawn
        AMX::Register("AddVehicleModel", n_AddVehicleModel);
        return true;
    }

    void onUnload() override
    {
        Log::Info(">> [CustomVehicles] Unloaded.");
    }
} plugin;
