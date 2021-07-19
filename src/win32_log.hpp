
#if _WIN32 && DEBUG_LOG
//#include "mbed_printf_implementation.h"
#include "mbed_printf_implementation.cpp"
void
Log(const char * format, ...)
{
    char buffer[512];
    va_list list;
    va_start(list,format);
    if (mbed_minimal_formatted_string(buffer, 512, format, list, 0))
    {
        OutputDebugStringA(buffer);
    }
}

// Global variable must be defined only once here
FPTR_Log LogHandler = Log;

#endif
