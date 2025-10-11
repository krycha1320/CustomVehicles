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
    PLUGIN_DATA_AMX_EXPORTS = 1
};

struct AMX_NATIVE_INFO { const char* name; cell (*func)(AMX*, cell*); };

void (*logprintf)(const char* format, ...) = nullptr;
int (*amx_Register_real)(AMX*, const AMX_NATIVE_INFO*, int) = nullptr;

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
        if (logprintf) logprintf("[CustomVehicles] ERROR: cannot open vehicles.j
