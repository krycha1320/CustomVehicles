#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <cstdarg> // dla va_start / va_end

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
    #define EXPORT extern "C" __declspec(dllexport)
#else
    #include <unistd.h>
    #define EXPORT extern "C"
#endif

// ==========================================================
// Minimalne definicje pluginu
// ==========================================================
typedef unsigned int cell;
typedef void* AMX;
#define AMX_NATIVE_CALL
#define PLUGIN_CALL
#define SUPPORTS_VERSION 1
#define SUPPORTS_AMX_NATIVES 2
#define AMX_ERR_NONE 0

// ==========================================================
// Struktura pojazdu
// ==========================================================
struct VehicleDef {
    int baseid = 0;
    int newid = 0;
    std::string dff;
    std::string txd;
};

std::vector<VehicleDef> g_Vehicles;

// Prosty logger (działa na każdym systemie)
void DefaultLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

void (*logprintf)(const char* format, ...) = DefaultLog;

// ==========================================================
// Zapis JSON do scriptfiles
// ==========================================================
void SaveVehiclesJSON() {
#ifdef _WIN32
    mkdir("scriptfiles", 0777);
#else
    mkdir("scriptfiles", 0777);
#endif

    std::ofstream file("scriptfiles/vehicles.json");
    if (!file.is_open()) {
        logprintf("[CustomVehicles] ERROR: Cannot open scriptfiles/vehicles.json!");
        return;
    }

    file << "[\n";
    for (size_t i = 0; i < g_Vehicles.size(); ++i) {
        file << "  {\n";
        file << "    \"baseid\": " << g_Vehicles[i].baseid << ",\n";
        file << "    \"newid\": " << g_Vehicles[i].newid << ",\n";
        file << "    \"dff\": \"" << g_Vehicles[i].dff << "\",\n";
        file << "    \"txd\": \"" << g_Vehicles[i].txd << "\"\n";
        file << "  }";
        if (i < g_Vehicles.size() - 1) file << ",";
        file << "\n";
    }
    file << "]";
    file.close();

    logprintf("[CustomVehicles] Saved %zu vehicles to scriptfiles/vehicles.json", g_Vehicles.size());
}

// ==========================================================
// Główna funkcja dodająca model
// ==========================================================
cell AMX_NATIVE_CALL AddVehicleModel(cell baseid, cell newid, const char* dff, const char* txd)
{
    if (!dff || !txd) {
        logprintf("[CustomVehicles] ERROR: missing file name!");
        return 0;
    }

    if (newid < 20000) {
        logprintf("[CustomVehicles] ERROR: invalid new ID (%d) must be >=20000", newid);
        return 0;
    }

    VehicleDef v;
    v.baseid = (int)baseid;
    v.newid = (int)newid;
    v.dff = dff;
    v.txd = txd;
    g_Vehicles.push_back(v);

    SaveVehiclesJSON();

    logprintf("[CustomVehicles] Added model: base %d → new %d (%s / %s)", baseid, newid, dff, txd);

    return 1;
}

// ==========================================================
// Symulacja interfejsu pluginu (bez amx.h)
// ==========================================================
EXPORT unsigned int PLUGIN_CALL Supports()
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

EXPORT bool PLUGIN_CALL Load(void**)
{
    logprintf(">> CustomVehicles plugin (no-SDK build) loaded successfully!");
    return true;
}

EXPORT void PLUGIN_CALL Unload()
{
    logprintf(">> CustomVehicles plugin unloaded.");
}

EXPORT int PLUGIN_CALL AmxLoad(AMX*)
{
    logprintf("[CustomVehicles] Dummy AMX load (no SDK).");
    return 0;
}

EXPORT int PLUGIN_CALL AmxUnload(AMX*)
{
    return AMX_ERR_NONE;
}
