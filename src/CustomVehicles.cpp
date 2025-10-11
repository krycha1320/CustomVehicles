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

struct AMX_NATIVE_INFO { const char* name; cell (*func)(AMX*, cell*); };

// wskaźniki do funkcji z serwera
void (*logprintf)(const char* format, ...) = nullptr;
int (*amx_Register_real)(AMX*, const AMX_NATIVE_INFO*, int) = nullptr;

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
// Główna funkcja logiki
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
// Native dla Pawn
// ==========================================================
cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params)
{
    return AddVehicleModel_Internal(params[1], params[2], "car.dff", "car.txd");
}

// ==========================================================
// Interfejs pluginu
// ==========================================================
EXPORT unsigned int PLUGIN_CALL Supports()
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

EXPORT bool PLUGIN_CALL Load(void** ppData)
{
    // dynamiczne mapowanie funkcji
    struct PluginData {
        void* data[16];
    };

    PluginData* pd = (PluginData*)ppData;
    logprintf = (void(*)(const char*, ...))pd->data[0];
    amx_Register_real = (int(*)(AMX*, const AMX_NATIVE_INFO*, int))pd->data[2];

    if (!logprintf) logprintf = DefaultLog;

    logprintf(">> CustomVehicles plugin (final build) loaded successfully!");
    return true;
}

EXPORT void PLUGIN_CALL Unload()
{
    if (logprintf) logprintf(">> CustomVehicles plugin unloaded.");
}

EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
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
        if (logprintf) logprintf("[CustomVehicles] ERROR: amx_Register not available!");
    }

    return 0;
}

EXPORT int PLUGIN_CALL AmxUnload(AMX*)
{
    return AMX_ERR_NONE;
}
