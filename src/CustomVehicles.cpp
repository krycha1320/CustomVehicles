#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir _mkdir
    #define EXPORT extern "C" __declspec(dllexport)
#else
    #include <dlfcn.h>
    #include <unistd.h>
    #define EXPORT extern "C"
#endif

// ==========================================================
// Minimalne definicje potrzebne do pracy pluginu
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

// Funkcja logująca (przypisywana przez serwer)
void (*logprintf)(const char* format, ...) = nullptr;

// ==========================================================
// Prosta funkcja zapisu JSON
// ==========================================================
void SaveVehiclesJSON() {
    mkdir("scriptfiles");
    std::ofstream file("scriptfiles/vehicles.json");
    if (!file.is_open()) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: cannot open scriptfiles/vehicles.json!");
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

    if (logprintf)
        logprintf("[CustomVehicles] Saved %zu vehicles to scriptfiles/vehicles.json", g_Vehicles.size());
}

// ==========================================================
// Główna funkcja – rejestracja pojazdu
// ==========================================================
cell AMX_NATIVE_CALL AddVehicleModel(cell baseid, cell newid, const char* dff, const char* txd)
{
    if (!dff || !txd) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: missing file name!");
        return 0;
    }
    if (newid < 20000) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: invalid new ID (%d) must be >=20000", newid);
        return 0;
    }

    VehicleDef v;
    v.baseid = (int)baseid;
    v.newid = (int)newid;
    v.dff = dff;
    v.txd = txd;
    g_Vehicles.push_back(v);

    SaveVehiclesJSON();

    if (logprintf)
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

EXPORT bool PLUGIN_CALL Load(void** ppData)
{
    // w prawdziwym pluginie tu byłoby ppData[PLUGIN_DATA_LOGPRINTF]
    logprintf = [](const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        printf("\n");
        va_end(args);
    };

    logprintf(">> CustomVehicles plugin (no-SDK build) loaded successfully!");
    return true;
}

EXPORT void PLUGIN_CALL Unload()
{
    if (logprintf) logprintf(">> CustomVehicles plugin unloaded.");
}

EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
    if (logprintf) logprintf("[CustomVehicles] Dummy AMX load (no SDK).");
    return 0;
}

EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
    return AMX_ERR_NONE;
}
