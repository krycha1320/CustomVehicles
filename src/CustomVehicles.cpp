#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <sys/stat.h>

// =============== minimalne typy z SA-MP ====================
typedef unsigned int cell;
typedef void* AMX;

#define AMX_ERR_NONE 0
#define PLUGIN_DATA_LOGPRINTF 0
#define PLUGIN_DATA_AMX_EXPORTS 1

#ifndef AMX_NATIVE_CALL
#define AMX_NATIVE_CALL
#endif

struct AMX_NATIVE_INFO { const char* name; cell (*func)(AMX*, cell*); };

// =============== globalne wskaźniki ========================
void (*logprintf)(const char*, ...) = nullptr;
void** ppAMXFunctions = nullptr;
std::vector<AMX*> g_AmxList;

// =============== proste logowanie ===========================
void DefaultLog(const char* fmt, ...) {
    va_list a; va_start(a, fmt);
    vprintf(fmt, a); printf("\n");
    va_end(a);
}

// =============== dane pojazdów ==============================
struct VehicleDef { int baseid, newid; std::string dff, txd; };
std::vector<VehicleDef> g_Vehicles;

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
    logprintf("[CustomVehicles] Saved %zu vehicles.", g_Vehicles.size());
}

void LoadVehicles() {
    std::ifstream f("scriptfiles/vehicles.txt");
    if (!f.is_open()) {
        logprintf("[CustomVehicles] No vehicles.txt found, skipping load.");
        return;
    }
    g_Vehicles.clear();
    int b,n; std::string d,t;
    while (f >> b >> n >> d >> t) g_Vehicles.push_back({b,n,d,t});
    f.close();
    logprintf("[CustomVehicles] Loaded %zu vehicles.", g_Vehicles.size());
}

// =============== native =====================================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX*, cell* p) {
    int baseid=(int)p[1], newid=(int)p[2];
    const char* dff=(const char*)p[3], *txd=(const char*)p[4];
    logprintf("[CustomVehicles] AddVehicleModel(%d,%d,%s,%s)",baseid,newid,dff,txd);
    g_Vehicles.push_back({baseid,newid,dff,txd});
    SaveVehicles();
    return 1;
}

// =============== API pluginu ================================
extern "C" unsigned int PLUGIN_CALL Supports() { return 1|2; }

extern "C" bool PLUGIN_CALL Load(void** ppData) {
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    ppAMXFunctions = (void**)ppData[PLUGIN_DATA_AMX_EXPORTS];
    if (!logprintf) logprintf = DefaultLog;
    logprintf(">> [CustomVehicles] Loaded (no-SDK mode)");
    LoadVehicles();
    return true;
}

extern "C" void PLUGIN_CALL Unload() {
    logprintf("[CustomVehicles] Unloaded.");
}

extern "C" int PLUGIN_CALL AmxLoad(AMX* amx) {
    static const AMX_NATIVE_INFO natives[] = {
        {"AddVehicleModel", n_AddVehicleModel},
        {nullptr,nullptr}
    };
    g_AmxList.push_back(amx);
    // w open.mp 1.4 nie ma amx_Register, ale Pawn sam widzi nativa z listy
    logprintf("[CustomVehicles] AmxLoad() – AddVehicleModel ready");
    return AMX_ERR_NONE;
}

extern "C" int PLUGIN_CALL AmxUnload(AMX* amx) {
    g_AmxList.erase(std::remove(g_AmxList.begin(), g_AmxList.end(), amx), g_AmxList.end());
    return AMX_ERR_NONE;
}
