#ifndef PLATFORM_H

#include <stdint.h>

#define Kilobytes(x) x*1024
#define Megabytes(x) x*1024*Kilobytes(1)
#define Gigabytes(x) x*1024*Megabytes(1)

#define Assert(exp) if (!(exp)) { *(volatile int *)0 = 0; } 
#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))
#define global_variable static
#define local_persist   static

#define BIT_SET(Byte,Bit) ((Byte) & (1<<Bit))

typedef int32_t     int32;
typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef uint64_t    uint64;
typedef int32       bool32;
typedef float       real32;
typedef double      real64;

#define PI 3.141684f

#if _WIN32 && DEBUG_LOG
typedef void (*FPTR_Log) (const char *, ...);
extern FPTR_Log LogHandler;
#else
#define Log(format, ...)
#endif

#define API __declspec(dllexport)

#define PLATFORM_H
#endif
