// ============================================================
//  Plugin: CustomVehicles (wersja bez SDK / bez inc)
//  Autor: Krystian Chmielewski (Puls Miasta)
//  Kompiluje się bez samp-plugin-sdk i bez amx.h
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <cstdarg>

// ============================================================
//  Minimalne typy i struktury
// ============================================================
typedef int cell;

// struktura AMX nie jest używana, ale musi istnieć
struct AMX { int dummy; };

// wskaźnik do logowania z SA-MP
typedef void (*logprintf_t)(const char* format, ...);
static logprintf_t logprintf = nullptr;

// ============================================================
//  Struktura pojazdu (nasze własne dane)
// ============================================================
struct VehicleDef {
    int baseid = 0;
    int newid = 0;
    std::string dff;
    std::string txd;
};

static std::vector<VehicleDef> g_Vehicles;

// ============================================================
//  Funkcje eksportowane do serwera SA-MP
// ============================================================

// Serwer wywołuje to, aby sprawdzić, co plugin obsługuje
extern "C" __declspec(dllexport) unsigned int Supports()
{
    return 0x0200; // SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES
}

// Ładowanie pluginu
extern "C" __declspec(dllexport) bool Load(void** ppData)
{
    // w SA-MP logprintf to ppData[0]
    logprintf = (logprintf_t)ppData[0];
    if (logprintf)
        logprintf(">> CustomVehicles (no SDK) loaded!");
    return true;
}

// Wyładowanie pluginu
extern "C" __declspec(dllexport) void Unload()
{
    if (logprintf)
        logprintf(">> CustomVehicles unloaded!");
}

// ============================================================
//  Funkcja dodająca nowy model pojazdu
// ============================================================
extern "C" __declspec(dllexport) int AddVehicleModel(int baseid, int newid, const char* dff, const char* txd)
{
    VehicleDef v;
    v.baseid = baseid;
    v.newid = newid;
    v.dff = dff ? dff : "";
    v.txd = txd ? txd : "";
    g_Vehicles.push_back(v);

    if (logprintf)
        logprintf("[CustomVehicles] Added vehicle (base %d -> new %d): %s / %s", baseid, newid, dff, txd);

    return 1;
}

// ============================================================
//  Zapis do JSON
// ============================================================
static void SaveVehiclesJSON()
{
    std::ofstream file("scriptfiles/vehicles.json");
    if (!file.is_open()) return;

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

    if (logprintf)
        logprintf("[CustomVehicles] Saved %zu vehicles to scriptfiles/vehicles.json", g_Vehicles.size());
}

// ============================================================
//  Załadowanie i rozładowanie AMX (puste, wymagane przez SA-MP)
// ============================================================
extern "C" __declspec(dllexport) int AmxLoad(AMX* amx) { return 0; }
extern "C" __declspec(dllexport) int AmxUnload(AMX* amx) { SaveVehiclesJSON(); return 0; }
