#include <std_include.hpp>

#ifdef INJECT_HOST_AS_LIB
#pragma comment(linker, "/base:0x160000000")
#else
#pragma comment(linker, "/base:0x140000000")
#endif

#ifndef INJECT_HOST_AS_LIB
#pragma bss_seg(".payload")
char payload_data[BINARY_PAYLOAD_SIZE];
#endif

