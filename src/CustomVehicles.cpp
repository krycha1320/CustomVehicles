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

typedef unsigned int cell;
typedef void* AMX;

#define AMX_NATIVE_CALL
#define PLUGIN_CALL
#define SUPPORTS_VERSION 1
#define SUPPORTS_AMX_NATIVES 2
#define AMX_ERR_NONE 0

enum PLUGIN_DATA {
    PLUGIN_DATA_LOGPRINTF = 0,
    PLUGIN_DATA_AMX_EXPORTS = 1,
    PLUGIN_DATA_REGISTER_NATIVE = 13
};

struct AMX_NATIVE_INFO { const char* name; cell (*func)(AMX*, cell*); };

void (*logprintf)(const char* format, ...) = nullptr;
int (*amx_Register_real)(AMX*, const AMX_NATIVE_INFO*, int) = nullptr;
void (*RegisterPawnNative)(const AMX_NATIVE_INFO*) = nullptr;

struct VehicleDef {
    int baseid;
    int newid;
    std::string dff;
    std::string txd;
};
std::vector<VehicleDef> g_Vehicles;

void DefaultLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

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

cell AddVehicleModel_Internal(int baseid, int newid, const char* dff, const char* txd) {
    if (!dff || !txd) return 0;
    if (newid < 20000) {
        if (logprintf) logprintf("[CustomVehicles] Invalid new ID (%d)", newid);
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

// =============== PAWN NATIVE ===============
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params) {
    // params: baseid, newid, dff[], txd[]
    char* dff = (char*)params[3];
    char* txd = (char*)params[4];
    return AddVehicleModel_Internal((int)params[1], (int)params[2], dff, txd);
}

// =============== PLUGIN CORE ===============
EXPORT unsigned int PLUGIN_CALL Supports() {
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

EXPORT bool PLUGIN_CALL Load(void** ppData) {
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    if (!logprintf) logprintf = DefaultLog;

    void** amx_exports = (void**)ppData[PLUGIN_DATA_AMX_EXPORTS];
    amx_Register_real = (int(*)(AMX*, const AMX_NATIVE_INFO*, int))amx_exports[0];

    RegisterPawnNative = (void(*)(const AMX_NATIVE_INFO*))ppData[PLUGIN_DATA_REGISTER_NATIVE];

    static const AMX_NATIVE_INFO natives[] = {
        {"AddVehicleModel", n_AddVehicleModel},
        {nullptr, nullptr}
    };

    if (RegisterPawnNative) {
        logprintf("[CustomVehicles] Detected open.mp (native registry ✅)");
        RegisterPawnNative(natives);
    }

    logprintf(">> CustomVehicles plugin loaded!");
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
    if (amx_Register_real) amx_Register_real(amx, natives, -1);
    return AMX_ERR_NONE;
}

EXPORT int PLUGIN_CALL AmxUnload(AMX*) {
    return AMX_ERR_NONE;
}
