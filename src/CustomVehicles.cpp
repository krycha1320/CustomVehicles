#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <cstdarg>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
    #define EXPORT extern "C" __declspec(dllexport)
#else
    #include <unistd.h>
    #define EXPORT extern "C"
#endif

// ============================================================
// Minimalne definicje dla SA-MP 0.3.DL (bez open.mp rozszerzeń)
// ============================================================
typedef unsigned int cell;
typedef void* AMX;

#define AMX_NATIVE_CALL
#define PLUGIN_CALL
#define SUPPORTS_VERSION 1
#define SUPPORTS_AMX_NATIVES 2
#define AMX_ERR_NONE 0

enum PLUGIN_DATA {
    PLUGIN_DATA_LOGPRINTF = 0,
    PLUGIN_DATA_AMX_EXPORTS = 1
};

struct AMX_NATIVE_INFO { const char* name; cell (*func)(AMX*, cell*); };

// ============================================================
// Globalne wskaźniki serwera
// ============================================================
void (*logprintf)(const char* format, ...) = nullptr;
int (*amx_Register_real)(AMX*, const AMX_NATIVE_INFO*, int) = nullptr;

// ============================================================
// Struktura modelu pojazdu
// ============================================================
struct VehicleDef {
    int baseid;
    int newid;
    std::string dff;
    std::string txd;
};
std::vector<VehicleDef> g_Vehicles;

// ============================================================
// Logger awaryjny
// ============================================================
void DefaultLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

// ============================================================
// Zapis do JSON
// ============================================================
void SaveVehiclesJSON() {
    mkdir("scriptfiles", 0777);

    std::ofstream file("scriptfiles/vehicles.json");
    if (!file.is_open()) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: cannot open vehicles.json!");
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

// ============================================================
// Główna logika dodawania pojazdu
// ============================================================
cell AddVehicleModel_Internal(int baseid, int newid, const char* dff, const char* txd) {
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

// ============================================================
// Pawn native: AddVehicleModel(baseid, newid, dff[], txd[])
// ============================================================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params) {
    // params[1] = baseid, params[2] = newid, params[3] = dff, params[4] = txd
    const char* dff = reinterpret_cast<const char*>(params[3]);
    const char* txd = reinterpret_cast<const char*>(params[4]);
    return AddVehicleModel_Internal((int)params[1], (int)params[2], dff, txd);
}

// ============================================================
// Interfejs pluginu
// ============================================================
EXPORT unsigned int PLUGIN_CALL Supports() {
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

EXPORT bool PLUGIN_CALL Load(void** ppData) {
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    if (!logprintf) logprintf = DefaultLog;

    void** amx_exports = (void**)ppData[PLUGIN_DATA_AMX_EXPORTS];
    amx_Register_real = (int(*)(AMX*, const AMX_NATIVE_INFO*, int))amx_exports[0];

    logprintf(">> CustomVehicles plugin (SA-MP 0.3.DL) loaded successfully!");
    return true;
}

EXPORT void PLUGIN_CALL Unload() {
    if (logprintf) logprintf(">> CustomVehicles plugin unloaded.");
}

EXPORT int PLUGIN_CALL AmxLoad(AMX* amx) {
    static const AMX_NATIVE_INFO natives[] = {
        {"AddVehicleModel", n_AddVehicleModel},
        {nullptr, nullptr}
    };
    if (amx_Register_real)
        amx_Register_real(amx, natives, -1);
    if (logprintf) logprintf("[CustomVehicles] Registered Pawn native: AddVehicleModel");
    return AMX_ERR_NONE;
}

EXPORT int PLUGIN_CALL AmxUnload(AMX*) {
    return AMX_ERR_NONE;
}
