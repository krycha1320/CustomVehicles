#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdarg>
#include <sys/stat.h>

// ====================== DEFINICJE PODSTAWOWE ======================
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

// ====================== GLOBALNE ======================
void (*logprintf)(const char*, ...) = nullptr;
std::vector<AMX*> g_AmxList;

struct VehicleDef {
    int baseid, newid;
    std::string dff, txd;
};
std::vector<VehicleDef> g_Vehicles;

// ====================== CONFIG ======================
std::string cdnUrl = "https://krycha1320.github.io/Samp/models/";

// ====================== LOGOWANIE ======================
void DefaultLog(const char* fmt, ...) {
    va_list a;
    va_start(a, fmt);
    vprintf(fmt, a);
    printf("\n");
    va_end(a);
}

// ====================== SAVE / LOAD ======================
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
#ifdef _WIN32
    _mkdir("scriptfiles");
#else
    mkdir("scriptfiles", 0777);
#endif
    std::ifstream f("scriptfiles/vehicles.txt");
    if (!f.is_open()) {
        if (logprintf) logprintf("[CustomVehicles] No vehicles.txt found, creating empty file.");
        std::ofstream create("scriptfiles/vehicles.txt");
        create.close();
        return;
    }

    g_Vehicles.clear();
    int b, n;
    std::string d, t;
    while (f >> b >> n >> d >> t)
        g_Vehicles.push_back({b, n, d, t});
    f.close();

    if (logprintf) {
        if (g_Vehicles.empty())
            logprintf("[CustomVehicles] vehicles.txt loaded but is empty.");
        else
            logprintf("[CustomVehicles] Loaded %zu vehicles.", g_Vehicles.size());
    }
}

// ====================== REGISTRACJA POJAZDÓW ======================
void RegisterVehiclesFromCDN() {
    if (g_Vehicles.empty()) {
        if (logprintf) logprintf("[CustomVehicles] No custom vehicles to register.");
        return;
    }

    for (auto& v : g_Vehicles) {
        std::string dffUrl = cdnUrl + v.dff;
        std::string txdUrl = cdnUrl + v.txd;

        if (logprintf) {
            logprintf("[CustomVehicles] Registering baseid=%d newid=%d", v.baseid, v.newid);
            logprintf("[CustomVehicles] Using DFF: %s", dffUrl.c_str());
            logprintf("[CustomVehicles] Using TXD: %s", txdUrl.c_str());
        }

        // zamiast AddVehicleModel -> AddSimpleModel (działa w open.mp client)
        std::string cmd = "AddSimpleModel(-1, " + std::to_string(v.baseid) + ", \"" + dffUrl + "\", \"" + txdUrl + "\");";
        if (logprintf)
            logprintf("[CustomVehicles] -> %s", cmd.c_str());
    }

    if (logprintf) logprintf("[CustomVehicles] All models queued for download.");
}

// ====================== NATIVE ======================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX*, cell* p) {
    int baseid = (int)p[1], newid = (int)p[2];
    const char* dff = (const char*)p[3];
    const char* txd = (const char*)p[4];

    g_Vehicles.push_back({baseid, newid, dff, txd});
    if (logprintf)
        logprintf("[CustomVehicles] AddVehicleModel(%d, %d, %s, %s)", baseid, newid, dff, txd);
    SaveVehicles();
    return 1;
}

// ====================== PLUGIN HOOKS ======================
EXPORT unsigned int PLUGIN_CALL Supports() {
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

EXPORT bool PLUGIN_CALL Load(void** ppData) {
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    if (!logprintf) logprintf = DefaultLog;

    logprintf(">> [CustomVehicles] Plugin initializing...");
    LoadVehicles();
    RegisterVehiclesFromCDN();
    logprintf(">> [CustomVehicles] Initialization complete!");
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
    if (logprintf) logprintf("[CustomVehicles] AmxLoad() – native ready.");
    return AMX_ERR_NONE;
}

EXPORT int PLUGIN_CALL AmxUnload(AMX* amx) {
    g_AmxList.erase(std::remove(g_AmxList.begin(), g_AmxList.end(), amx), g_AmxList.end());
    return AMX_ERR_NONE;
}
