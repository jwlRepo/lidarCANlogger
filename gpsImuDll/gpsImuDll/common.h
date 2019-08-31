#pragma once
#pragma comment (lib, "ws2_32.lib")

#ifdef GPSIMUDLL_EXPORTS
#define DllExport __declspec(dllexport)
#else
#define DllExport __declspec(dllimport)
#endif

unsigned long int bytestoint(const std::string& s, bool flip);
std::string toHex(const std::string&, bool);
int64_t gettime();
SOCKET makeSock(std::string&, int, int);
