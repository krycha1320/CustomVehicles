// ============================================================
//  Plugin: CustomVehicles (cross-platform 32-bit build)
//  Autor: Krystian Chmielewski / Puls Miasta
//  Działa bez SDK, kompiluje się na Windows (.dll) i Linux (.so)
// ============================================================

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #define EXPORT extern "C" __declspec(dllexport)
#else
    #include <dlfcn.h>
    #include <unistd.h>
    #include <sys/stat.h> // <── DODANE, aby działało mkdir() na Linuxie
    #define EXPORT extern "C" __attribute__((visibility("default")))
#endif

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdarg>

// ============================================================
//  Minimalne typy i struktury AMX (wymagane przez serwer SA-MP)
// ============================================================
typedef int cell;
struct AMX { int dummy; };

// logprintf – wskaźnik na funkcję logowania SA-MP
typedef void (*logprintf_t)(const char* format, ...);
static logprintf_t logprintf = nullptr;

// ============================================================
//  Definicja struktury pojazdu
// ============================================================
struct VehicleDef {
    int baseid = 0;
    int newid = 0;
    std::string dff;
    std::string txd;
};

// globalna lista pojazdów
static std::vector<VehicleDef> g_Vehicles;

// ============================================================
//  Pomocnicza funkcja logowania
// ============================================================
static void Log(const char* fmt, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (logprintf)
        logprintf("%s", buffer);
    else
        std::cout << buffer << std::endl;
}

// ============================================================
//  Funkcje eksportowane przez plugin (dla SA-MP/open.mp)
// ============================================================

// Informacja o kompatybilności pluginu
EXPORT unsigned int Supports()
{
    return 0x0200; // SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES
}

// Ładowanie pluginu
EXPORT bool Load(void** ppData)
{
#ifdef _WIN32
    logprintf = (logprintf_t)ppData[0];
#endif

    Log(">> CustomVehicles plugin loaded successfully (cross-platform build).");
    return true;
}

// Wyładowanie pluginu
EXPORT void Unload()
{
    Log(">> CustomVehicles plugin unloaded.");
}

// ============================================================
//  Funkcja dodająca nowy model pojazdu
// ============================================================
EXPORT int AddVehicleModel(int baseid, int newid, const char* dff, const char* txd)
{
    VehicleDef v;
    v.baseid = baseid;
    v.newid = newid;
    v.dff = (dff ? dff : "");
    v.txd = (txd ? txd : "");
    g_Vehicles.push_back(v);

    Log("[CustomVehicles] Added vehicle model: base %d → new %d (%s / %s)",
        baseid, newid, v.dff.c_str(), v.txd.c_str());

    return 1;
}

// ============================================================
//  Zapis pojazdów do pliku JSON (scriptfiles/vehicles.json)
// ============================================================
static void SaveVehiclesJSON()
{
#ifdef _WIN32
    CreateDirectoryA("scriptfiles", nullptr);
#else
    mkdir("scriptfiles", 0777);
#endif

    std::ofstream file("scriptfiles/vehicles.json");
    if (!file.is_open())
    {
        Log("[CustomVehicles] ERROR: Cannot open scriptfiles/vehicles.json for writing.");
        return;
    }

    file << "[\n";
    for (size_t i = 0; i < g_Vehicles.size(); ++i)
    {
        const auto& v = g_Vehicles[i];
        file << "  { \"baseid\": " << v.baseid
             << ", \"newid\": " << v.newid
             << ", \"dff\": \"" << v.dff
             << "\", \"txd\": \"" << v.txd << "\" }";
        if (i + 1 < g_Vehicles.size()) file << ",";
        file << "\n";
    }
    file << "]";
    file.close();

    Log("[CustomVehicles] Saved %zu vehicles to scriptfiles/vehicles.json", g_Vehicles.size());
}

// ============================================================
//  Wymagane funkcje AMX (nieużywane, ale muszą istnieć)
// ============================================================
EXPORT int AmxLoad(AMX* amx)
{
    Log("[CustomVehicles] AMX loaded.");
    return 0;
}

EXPORT int AmxUnload(AMX* amx)
{
    SaveVehiclesJSON();
    Log("[CustomVehicles] AMX unloaded and data saved.");
    return 0;
}
