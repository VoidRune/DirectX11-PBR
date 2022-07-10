#pragma once
#include <Windows.h>
#include <stdio.h>

#ifdef _DEBUG
#define LOG_INIT()				AllocConsole(); \
								SetWindowPos(GetConsoleWindow(), 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER); \
								AttachConsole(GetCurrentProcessId()); \
								freopen_s((FILE**)(stdout), "CON", "w", stdout)

#define LOG_INFO(...) 	        printf("INFO: " __VA_ARGS__); printf("\n")
#define LOG_WARN(...) 	        printf("WARN: " __VA_ARGS__); printf("\n")
#define LOG_ERROR(...) 	        printf("ERROR: " __VA_ARGS__); printf("\n")
#define LOG_FATAL(...) 	        printf("FATAL: " __VA_ARGS__); printf("\n")

#define LOG_CORE_INFO(...) 	    printf("CORE INFO: " __VA_ARGS__); printf("\n")
#define LOG_CORE_WARN(...) 	    printf("CORE WARN: " __VA_ARGS__); printf("\n")
#define LOG_CORE_ERROR(...) 	printf("CORE ERROR: " __VA_ARGS__); printf("\n")
#define LOG_CORE_FATAL(...) 	printf("CORE FATAL: " __VA_ARGS__); printf("\n")

#define LOG_IF_INFO(x, ...) 	        if(x) {printf("INFO: " __VA_ARGS__); printf("\n");}
#define LOG_IF_WARN(x, ...) 	        if(x) {printf("WARN: " __VA_ARGS__); printf("\n");}
#define LOG_IF_ERROR(x, ...) 	        if(x) {printf("ERROR: " __VA_ARGS__); printf("\n");}
#define LOG_IF_FATAL(x, ...) 	        if(x) {printf("FATAL: " __VA_ARGS__); printf("\n");}

#define LOG_IF_CORE_INFO(x, ...) 	        if(x) {printf("CORE INFO: " __VA_ARGS__); printf("\n");}
#define LOG_IF_CORE_WARN(x, ...) 	        if(x) {printf("CORE WARN: " __VA_ARGS__); printf("\n");}
#define LOG_IF_CORE_ERROR(x, ...) 	        if(x) {printf("CORE ERROR: " __VA_ARGS__); printf("\n");}
#define LOG_IF_CORE_FATAL(x, ...) 	        if(x) {printf("CORE FATAL: " __VA_ARGS__); printf("\n");}

#else

#define LOG_INIT()

#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_FATAL(...)

#define LOG_CORE_INFO(...)
#define LOG_CORE_WARN(...)
#define LOG_CORE_ERROR(...)
#define LOG_CORE_FATAL(...)

#define LOG_IF_INFO(x, ...)
#define LOG_IF_WARN(x, ...)
#define LOG_IF_ERROR(x, ...)
#define LOG_IF_FATAL(x, ...)

#define LOG_IF_CORE_INFO(x, ...)
#define LOG_IF_CORE_WARN(x, ...)
#define LOG_IF_CORE_ERROR(x, ...)
#define LOG_IF_CORE_FATAL(x, ...)

#endif