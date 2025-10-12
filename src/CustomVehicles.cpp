#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdarg>
#include <sys/stat.h>

// ====================== DEFINICJE MINIMALNE ======================
typedef unsigned int cell;
typedef void* AMX;

#define AMX_ERR_NONE 0
#define PLUGIN_DATA_LOGPRINTF 0
#define PLUGIN_DATA_AMX_EXPORTS 1
#define SUPPORTS_VERSION 1
#define SUPPORTS_AMX_NATIVES 2

#ifndef AMX_NATIVE_CALL
#define AMX_NATIVE_CALL
#endif

#ifndef PLUGIN_CALL
#define PLUGIN_CALL
#endif

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
    #define EXPORT extern "C" __declspec(dllexport)
#else
    #define EXPORT extern "C"
#endif

struct AMX_NATIVE_INFO { const char* name; cell (*func)(AMX*, cell*); };

// ====================== GLOBALNE ZMIENNE ======================
void (*logprintf)(const char*, ...) = nullptr;
void** ppAMXFunctions = nullptr;
std::vector<AMX*> g_AmxList;

// ====================== LOGOWANIE ======================
void DefaultLog(const char* fmt, ...) {
    va_list a;
    va_start(a, fmt);
    vprintf(fmt, a);
    printf("\n");
    va_end(a);
}

// ====================== STRUKTURA POJAZDU ======================
struct VehicleDef {
    int baseid, newid;
    std::string dff, txd;
};

std::vector<VehicleDef> g_Vehicles;

// ====================== FUNKCJE ======================
void SaveVehicles() {
#ifdef _WIN32
    _mkdir("scriptfiles");
#else
    mkdir("scriptfiles", 0777);
#endif
    std::ofstream f("scriptfiles/vehicles.txt");
    for (auto& v : g_Vehicles)
        f << v.baseid << " " << v.newid << " " << v.dff << " " << v.txd << "\n";
    f.close();
    if (logprintf) logprintf("[CustomVehicles] Saved %zu vehicles.", g_Vehicles.size());
}

void LoadVehicles() {
    std::ifstream f("scriptfiles/vehicles.txt");
    if (!f.is_open()) {
        if (logprintf) logprintf("[CustomVehicles] No vehicles.txt found, skipping load.");
        return;
    }
    g_Vehicles.clear();
    int b, n;
    std::string d, t;
    while (f >> b >> n >> d >> t)
        g_Vehicles.push_back({b, n, d, t});
    f.close();
    if (logprintf) logprintf("[CustomVehicles] Loaded %zu vehicles.", g_Vehicles.size());
}

// ====================== NATIVE ======================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX*, cell* p) {
    int baseid = (int)p[1], newid = (int)p[2];
    const char* dff = (const char*)p[3];
    const char* txd = (const char*)p[4];

    if (logprintf)
        logprintf("[CustomVehicles] AddVehicleModel(%d, %d, %s, %s)", baseid, newid, dff, txd);

    g_Vehicles.push_back({baseid, newid, dff, txd});
    SaveVehicles();
    return 1;
}

// ====================== API PLUGINU ======================
EXPORT unsigned int PLUGIN_CALL Supports() {
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

EXPORT bool PLUGIN_CALL Load(void** ppData) {
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    if (!logprintf) logprintf = DefaultLog;

    ppAMXFunctions = (void**)ppData[PLUGIN_DATA_AMX_EXPORTS];

    logprintf(">> [CustomVehicles] Loaded (no-SDK mode)");
    LoadVehicles();
    return true;
}

EXPORT void PLUGIN_CALL Unload() {
    if (logprintf) logprintf("[CustomVehicles] Unloaded.");
}

EXPORT int PLUGIN_CALL AmxLoad(AMX* amx) {
    static const AMX_NATIVE_INFO natives[] = {
        {"AddVehicleModel", n_AddVehicleModel},
        {nullptr, nullptr}
    };
    g_AmxList.push_back(amx);
    if (logprintf) logprintf("[CustomVehicles] AmxLoad() -> AddVehicleModel ready.");
    return AMX_ERR_NONE;
}

EXPORT int PLUGIN_CALL AmxUnload(AMX* amx) {
    g_AmxList.erase(std::remove(g_AmxList.begin(), g_AmxList.end(), amx), g_AmxList.end());
    return AMX_ERR_NONE;
}
