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

// ==========================================================
// Minimalne definicje pluginu (bez SDK)
// ==========================================================
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
    PLUGIN_DATA_REGISTER_NATIVE = 13  // open.mp native registry
};

struct AMX_NATIVE_INFO { const char* name; cell (*func)(AMX*, cell*); };

// wskaźniki do funkcji serwera
void (*logprintf)(const char* format, ...) = nullptr;
int (*amx_Register_real)(AMX*, const AMX_NATIVE_INFO*, int) = nullptr;
void (*RegisterPawnNative)(const AMX_NATIVE_INFO*) = nullptr;

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

// ==========================================================
// Logger awaryjny
// ==========================================================
void DefaultLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

// ==========================================================
// Zapis JSON
// ==========================================================
void SaveVehiclesJSON()
{
    mkdir("scriptfiles", 0777);

    std::ofstream file("scriptfiles/vehicles.json");
    if (!file.is_open()) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: Cannot open vehicles.json!");
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
// Główna logika
// ==========================================================
cell AddVehicleModel_Internal(cell baseid, cell newid, const char* dff, const char* txd)
{
    if (!dff || !txd) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: missing file name!");
        return 0;
    }

    if (newid < 20000) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: invalid new ID (%d)", newid);
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
// Native Pawn
// ==========================================================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params)
{
    const char* dff = "car.dff";
    const char* txd = "car.txd";
    return AddVehicleModel_Internal(params[1], params[2], dff, txd);
}

// ==========================================================
// Plugin interfejs
// ==========================================================
EXPORT unsigned int PLUGIN_CALL Supports()
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

EXPORT bool PLUGIN_CALL Load(void** ppData)
{
    logprintf = (void(*)(const char*, ...))ppData[PLUGIN_DATA_LOGPRINTF];
    if (!logprintf) logprintf = DefaultLog;

    void** amx_exports = (void**)ppData[PLUGIN_DATA_AMX_EXPORTS];
    amx_Register_real = (int(*)(AMX*, const AMX_NATIVE_INFO*, int))amx_exports[0]; // 0 = amx_Register

    RegisterPawnNative = (void(*)(const AMX_NATIVE_INFO*))ppData[PLUGIN_DATA_REGISTER_NATIVE];

    if (RegisterPawnNative) {
        logprintf("[CustomVehicles] Detected open.mp -> Using RegisterPawnNative ✅");
        static const AMX_NATIVE_INFO natives[] = {
            {"AddVehicleModel", n_AddVehicleModel},
            {nullptr, nullptr}
        };
        RegisterPawnNative(natives);
    } else {
        logprintf("[CustomVehicles] Using local fallback amx_Register_local ✅");
    }

    logprintf(">> CustomVehicles plugin (universal) loaded successfully!");
    return true;
}

EXPORT void PLUGIN_CALL Unload()
{
    if (logprintf) logprintf(">> CustomVehicles plugin unloaded.");
}

EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
    if (RegisterPawnNative) {
        if (logprintf) logprintf("[CustomVehicles] AmxLoad() skipped (open.mp handles natives automatically).");
        return AMX_ERR_NONE;
    }

    static const AMX_NATIVE_INFO natives[] = {
        {"AddVehicleModel", n_AddVehicleModel},
        {nullptr, nullptr}
    };

    if (amx_Register_real)
    {
        amx_Register_real(amx, natives, -1);
        if (logprintf) logprintf("[CustomVehicles] Registered Pawn native: AddVehicleModel");
    }
    else
    {
        if (logprintf) logprintf("[CustomVehicles] WARNING: amx_Register pointer invalid!");
    }

    return AMX_ERR_NONE;
}

EXPORT int PLUGIN_CALL AmxUnload(AMX*)
{
    return AMX_ERR_NONE;
}
