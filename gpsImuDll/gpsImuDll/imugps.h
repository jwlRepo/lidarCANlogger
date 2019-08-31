#pragma once
#include "common.h"

class DllExport imugps {
	bool running = false;
	std::thread axioThread;
	SOCKET axioSock;
	std::string IMU_IP_ADDR = "169.254.0.34";
	int IMU_PORT = 4000;
	int TIMEOUT = 8000;

public:
	imugps();
	imugps(const std::string&, int);
	imugps(const std::string&, int, int);
	~imugps();
	int connect();
	void startLog(const std::string&);
	void stopLog();
	
private:
	void axioToThread(const std::string&);
	int isGPS(const std::string&);
	std::vector<long double> gpsDecode(const std::string&, int);
	int isIMU(const std::string&);
	std::vector<long double> imuDecode(const std::string&, int);
};