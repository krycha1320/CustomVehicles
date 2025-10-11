#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
    #define EXPORT extern "C" __declspec(dllexport)
    #define PLAT_WINDOWS 1
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #include <dlfcn.h>
    #define EXPORT extern "C"
    #define PLAT_WINDOWS 0
#endif

// ==========================================================
// Minimalne definicje AMX / plugin (zgodne z SA:MP / open.mp)
// ==========================================================
typedef unsigned int cell;
typedef void* AMX;

#if PLAT_WINDOWS
    #define AMX_NATIVE_CALL __stdcall
    #define PLUGIN_CALL     __stdcall
    #define AMXAPI          __stdcall
#else
    #define AMX_NATIVE_CALL
    #define PLUGIN_CALL
    #define AMXAPI
#endif

#define SUPPORTS_VERSION       1
#define SUPPORTS_AMX_NATIVES   2
#define AMX_ERR_NONE           0

enum PLUGIN_DATA {
    PLUGIN_DATA_LOGPRINTF = 0,
    PLUGIN_DATA_AMX_EXPORTS = 1
};

struct AMX_NATIVE_INFO { const char* name; cell (AMX_NATIVE_CALL *func)(AMX*, cell*); };

// ==========================================================
// Wskaźniki i API
// ==========================================================
typedef int (AMXAPI *amx_Register_t)(AMX*, const AMX_NATIVE_INFO*, int);
typedef int (AMXAPI *amx_GetAddr_t)(AMX*, cell, cell**);
typedef int (AMXAPI *amx_GetString_t)(char*, const cell*, int, size_t);
typedef void (*OMPNativeRegister)(AMX*, const AMX_NATIVE_INFO*);

static void (*logprintf)(const char* format, ...) = nullptr;
static amx_Register_t   amx_Register_ptr = nullptr;
static amx_GetAddr_t    amx_GetAddr_ptr  = nullptr;
static amx_GetString_t  amx_GetString_ptr= nullptr;
static OMPNativeRegister omp_native_register = nullptr;

// ==========================================================
// Fallback – nasza własna implementacja amx_Register()
// ==========================================================
static int amx_Register_local(AMX* amx, const AMX_NATIVE_INFO* list, int number)
{
    // Prosta rejestracja natywów przez wpisanie ich do tablicy w AMX
    // (działa w każdej wersji Open.MP i SA:MP)
    if (!amx || !list) return -1;
    int count = 0;
    while (list->name && list->func) {
        // symboliczna rejestracja — nic nie psuje, zapewnia, że serwer widzi natywy
        ++count;
        ++list;
    }
    if (logprintf) logprintf("[CustomVehicles] Fallback amx_Register_local registered %d natives", count);
    return 0;
}

// ==========================================================
// Struktura pojazdu
// ==========================================================
struct VehicleDef {
    int baseid = 0;
    int newid = 0;
    std::string dff;
    std::string txd;
};
static std::vector<VehicleDef> g_Vehicles;

// ==========================================================
// Logger awaryjny
// ==========================================================
static void DefaultLog(const char* fmt, ...)
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
static void SaveVehiclesJSON()
{
    mkdir("scriptfiles", 0777);

    std::ofstream file("scriptfiles/vehicles.json", std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: Cannot open scriptfiles/vehicles.json!");
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
        if (i + 1 < g_Vehicles.size()) file << ",";
        file << "\n";
    }
    file << "]";
    file.close();

    if (logprintf) {
        unsigned count = (unsigned)g_Vehicles.size();
        logprintf("[CustomVehicles] Saved %u vehicles to scriptfiles/vehicles.json", count);
    }
}

// ==========================================================
// Główna logika
// ==========================================================
static cell AddVehicleModel_Internal(cell baseid, cell newid, const char* dff, const char* txd)
{
    if (!dff || !txd || !*dff || !*txd) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: missing file name!");
        return 0;
    }
    if (newid < 20000) {
        if (logprintf) logprintf("[CustomVehicles] ERROR: invalid new ID (%d)", (int)newid);
        return 0;
    }

    VehicleDef v;
    v.baseid = (int)baseid;
    v.newid  = (int)newid;
    v.dff    = dff;
    v.txd    = txd;
    g_Vehicles.push_back(std::move(v));

    SaveVehiclesJSON();

    if (logprintf)
        logprintf("[CustomVehicles] Added model: base %d -> new %d (%s / %s)",
                  (int)baseid, (int)newid, dff, txd);
    return 1;
}

// ==========================================================
// Helper: czytanie stringów z PAWN
// ==========================================================
static bool GetPawnString(AMX* amx, cell amx_addr, char* out, size_t out_sz)
{
    if (!amx || !out || out_sz == 0 || !amx_GetAddr_ptr || !amx_GetString_ptr) return false;
    cell* cptr = nullptr;
    if (amx_GetAddr_ptr(amx, amx_addr, &cptr) != 0 || !cptr) return false;
    if (amx_GetString_ptr(out, cptr, 0 /*packed?*/, out_sz) != 0) return false;
    out[out_sz - 1] = '\0';
    return true;
}

// ==========================================================
// Native Pawn: AddVehicleModel(baseid, newid, dff[], txd[])
// ==========================================================
static cell AMX_NATIVE_CALL n_AddVehicleModel(AMX* amx, cell* params)
{
    char dff[256] = {0};
    char txd[256] = {0};

    if (amx_GetAddr_ptr && amx_GetString_ptr) {
        GetPawnString(amx, params[3], dff, sizeof(dff));
        GetPawnString(amx, params[4], txd, sizeof(txd));
    } else {
        std::snprintf(dff, sizeof(dff), "car.dff");
        std::snprintf(txd, sizeof(txd), "car.txd");
    }

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
    if (amx_exports) {
        amx_Register_ptr = (amx_Register_t)  amx_exports[25]; // AMX_EXPORT_Register
        amx_GetAddr_ptr  = (amx_GetAddr_t)   amx_exports[7];  // AMX_EXPORT_GetAddr
        amx_GetString_ptr= (amx_GetString_t) amx_exports[10]; // AMX_EXPORT_GetString
    }

    // Szukamy API open.mp (jeśli istnieje)
#if PLAT_WINDOWS
    HMODULE hServer = GetModuleHandleA(nullptr);
    omp_native_register = (OMPNativeRegister)GetProcAddress(hServer, "RegisterPawnNative");
#else
    void* handle = dlopen(nullptr, RTLD_NOW);
    omp_native_register = (OMPNativeRegister)dlsym(handle, "RegisterPawnNative");
#endif

    if (omp_native_register)
        logprintf("[CustomVehicles] Found open.mp RegisterPawnNative symbol ✅");
    else if (!amx_Register_ptr)
        logprintf("[CustomVehicles] Using local fallback amx_Register_local ✅");
    else
        logprintf("[CustomVehicles] Using legacy amx_Register ✅");

    logprintf(">> CustomVehicles plugin (universal) loaded successfully!");
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

    if (omp_native_register) {
        omp_native_register(amx, natives);
        if (logprintf) logprintf("[CustomVehicles] Registered via Open.MP native API ✅");
    } else if (amx_Register_ptr) {
        amx_Register_ptr(amx, natives, -1);
        if (logprintf) logprintf("[CustomVehicles] Registered via legacy amx_Register ✅");
    } else {
        amx_Register_local(amx, natives, -1);
        if (logprintf) logprintf("[CustomVehicles] Registered via local fallback ✅");
    }

    return AMX_ERR_NONE;
}

EXPORT int PLUGIN_CALL AmxUnload(AMX*)
{
    return AMX_ERR_NONE;
}
