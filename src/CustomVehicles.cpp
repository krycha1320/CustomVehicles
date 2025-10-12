#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdarg>
#include <sys/stat.h>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
#endif

// =============================================================
// Minimalne definicje AMX / open.mp API
// =============================================================
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

struct AMX_NATIVE_INFO {
    const char* name;
    cell (*func)(AMX*, cell*);
};

// =============================================================
// Globalne zmienne i funkcje AMX
// =============================================================
void *pAMXFunctions = nullptr;
void (*logprintf)(const char* format, ...) = nullptr;

#define amx_Register ((int (*)(AMX*, const AMX_NATIVE_INFO*, int))(((void**)pAMXFunctions)[12]))

// =============================================================
// Minimalna wersja amx_GetString
// =============================================================
static void amx_GetString(char *dest, cell addr, int /*use_wchar*/, size_t size)
{
    if (!addr || !dest || size == 0) return;
    const char *src = (const char*)addr;
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

// =============================================================
// Logger awaryjny (fallback)
// =============================================================
void DefaultLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

// =============================================================
// Dane o pojazdach
// =============================================================
struct VehicleDef {
    int baseid;
    int newid;
    std::string dff;
    std::string txd;
};
std::vector<VehicleDef> g_Vehicles;

// =============================================================
// Zapis pojazdów do JSON
// =============================================================
void SaveVehiclesJSON() {
#ifdef _WIN32
    _mkdir("scriptfiles");
#else
    mkdir("scriptfiles", 0777);
#endif

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

// =============================================================
// Główna funkcja dodawania pojazdu
// =============================================================
cell AddVehicleModel_Internal(int baseid, int newid, const char* dff, const char* txd) {
    if (!dff || !txd) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: Missing DFF or TXD name!");
        return 0;
    }

    VehicleDef v { baseid, newid, dff, txd };
    g_Vehicles.push_back(v);
    SaveVehiclesJSON();

    if (logprintf)
        logprintf("[CustomVehicles] Added vehicle model: base %d → new %d (%s / %s)",
                  baseid, newid, dff, txd);
    return 1;
}

// =============================================================
// Pawn native: AddVehicleModel(baseid, newid, dff[], txd[])
// =============================================================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params) {
    int baseid = (int)params[1];
    int newid  = (int)params[2];

    char dff[64], txd[64];
    amx_GetString(dff, params[3], 0, sizeof(dff));
    amx_GetString(txd, params[4], 0, sizeof(txd));

    return AddVehicleModel_Internal(baseid, newid, dff, txd);
}

// =============================================================
// INTERFEJS PLUGINU (open.mp / SA:MP kompatybilny)
// =============================================================
extern "C" __attribute__((visibility("default"))) unsigned int Supports() {
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

extern "C" __attribute__((visibility("default"))) bool Load(void **ppData) {
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    if (!logprintf) logprintf = DefaultLog;

    logprintf(">> [CustomVehicles] Load() called - plugin initialized!");
    return true;
}

extern "C" __attribute__((visibility("default"))) void Unload() {
    if (logprintf) logprintf(">> [CustomVehicles] Unload() called - plugin shutting down!");
}

extern "C" __attribute__((visibility("default"))) int AmxLoad(AMX *amx) {
    if (logprintf) logprintf("[CustomVehicles] AmxLoad() called - registering natives...");

    static const AMX_NATIVE_INFO natives[] = {
        {"AddVehicleModel", n_AddVehicleModel},
        {nullptr, nullptr}
    };

    amx_Register(amx, natives, -1);

    if (logprintf) logprintf("[CustomVehicles] Registered Pawn native: AddVehicleModel");
    return AMX_ERR_NONE;
}

extern "C" __attribute__((visibility("default"))) int AmxUnload(AMX *amx) {
    if (logprintf) logprintf("[CustomVehicles] AmxUnload() called!");
    return AMX_ERR_NONE;
}
