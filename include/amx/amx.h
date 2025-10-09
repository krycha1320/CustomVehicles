// amx.h - SA:MP Plugin SDK (poprawiona wersja dla CustomVehicles)
// ===============================================================
// Dodano <cstddef> aby size_t działało poprawnie w GCC/Linux
// ===============================================================

#pragma once

#include <cstddef>   // ✅ potrzebne dla size_t
#include <stdint.h>  // standardowe typy int32_t, uint32_t

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AMX_H_INCLUDED
#define AMX_H_INCLUDED

/* calling convention for exported functions */
#if defined __GNUC__
  #define AMXAPI
  #define AMXEXPORT
#elif defined WIN32 || defined _WIN32 || defined __WIN32__
  #define AMXAPI __stdcall
  #define AMXEXPORT __declspec(dllexport)
#else
  #define AMXAPI
  #define AMXEXPORT
#endif

/* types */
typedef int32_t  cell;
typedef uint32_t ucell;

#define AMX_USERNUM     4
#define sEXPMAX         19
#define AMX_FLAG_DEBUG  0x02

/* forward declarations */
typedef struct tagAMX AMX;
typedef struct tagAMX_HEADER AMX_HEADER;

/* structure for native functions */
typedef struct tagAMX_NATIVE_INFO {
  const char *name;
  int (AMXAPI *func)(AMX *amx, cell *params);
} AMX_NATIVE_INFO;

/* structure for AMX */
struct tagAMX {
  unsigned char *base;
  unsigned char *data;
  unsigned char *code;
  int            cip;
  cell           hea, hlw, stk, stp;
  int            flags;
  int            curline;
  void          *userdata[AMX_USERNUM];
  void          *callback;
  void          *debug;
  int            error;
  int            paramcount;
  cell           pri;
  cell           alt;
};

/* structure for AMX file header */
struct tagAMX_HEADER {
  int32_t size;
  uint16_t magic;
  char file_version;
  char amx_version;
  int32_t flags;
  int16_t defsize;
  int32_t cod;
  int32_t dat;
  int32_t hea;
  int32_t stp;
  int32_t cip;
  int32_t publics;
  int32_t natives;
  int32_t libraries;
  int32_t pubvars;
  int32_t tags;
  int32_t nametable;
};

/* =====================================================
   Error codes
   ===================================================== */
enum {
  AMX_ERR_NONE,
  AMX_ERR_EXIT,
  AMX_ERR_ASSERT,
  AMX_ERR_STACKERR,
  AMX_ERR_BOUNDS,
  AMX_ERR_MEMACCESS,
  AMX_ERR_INVINSTR,
  AMX_ERR_STACKLOW,
  AMX_ERR_HEAPLOW,
  AMX_ERR_CALLBACK,
  AMX_ERR_NATIVE,
  AMX_ERR_DIVIDE,
  AMX_ERR_SLEEP,
  AMX_ERR_INVSTATE
};

/* =====================================================
   Function prototypes
   ===================================================== */

int AMXAPI amx_Init(AMX *amx, void *program);
int AMXAPI amx_Cleanup(AMX *amx);
int AMXAPI amx_Exec(AMX *amx, cell *retval, int index);
int AMXAPI amx_FindPublic(AMX *amx, const char *name, int *index);
int AMXAPI amx_Register(AMX *amx, const AMX_NATIVE_INFO *nativelist, int number);
int AMXAPI amx_StrLen(const cell *cstring, int *length);

/* Funkcje z poprawką — dodano size_t */
int AMXAPI amx_GetString(char *dest, const cell *source, int use_wchar, size_t size);
int AMXAPI amx_SetString(cell *dest, const char *source, int pack, int use_wchar, size_t size);

int AMXAPI amx_Allot(AMX *amx, int cells, cell *amx_addr, cell **phys_addr);
int AMXAPI amx_Release(AMX *amx, cell amx_addr);

/* =====================================================
   Helper macros
   ===================================================== */

#define amx_StrParam(amx, param, result) \
    do { \
        cell *amx_cstr; \
        int amx_length; \
        amx_GetAddr((amx), (param), &amx_cstr); \
        amx_StrLen(amx_cstr, &amx_length); \
        if (amx_length > 0 && ((result) = (char*)alloca(amx_length + 1))) \
            amx_GetString((result), amx_cstr, 0, amx_length + 1); \
        else (result) = NULL; \
    } while (0)

/* =====================================================
   AMX utility functions
   ===================================================== */

int AMXAPI amx_GetAddr(AMX *amx, cell amx_addr, cell **phys_addr);
int AMXAPI amx_NameLength(AMX *amx, int *length);
int AMXAPI amx_NumPublics(AMX *amx, int *number);
int AMXAPI amx_NumNatives(AMX *amx, int *number);
int AMXAPI amx_NumTags(AMX *amx, int *number);
int AMXAPI amx_NumPubVars(AMX *amx, int *number);

#endif /* AMX_H_INCLUDED */

#ifdef __cplusplus
}
#endif
