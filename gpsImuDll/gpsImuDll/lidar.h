#pragma once
#include "common.h"
#include <vector>

class DllExport lidar {
	bool running = false;
	std::thread lidarThread;
	SOCKET lidarSock;
	char endChar = (char)3;
	std::string LIDAR_IP_ADDR = "169.254.75.136";
	int LIDAR_PORT = 2111;
	int TIMEOUT = 100;
	int planes = 2;
	std::ofstream lidarlogfile;
	std::ofstream lidarsettingfile;
	int length;

public:
	lidar();
	~lidar();
	lidar(const std::string&, int);
	lidar(const std::string&, int, int);
	int connect();
	void startLog(const std::string&);
	void stopLog();
	void setPlane(int);

private:
	void lidarToThread(const std::string& filename);
	ptrdiff_t logSpecifics(std::vector<std::string>&, ptrdiff_t, std::string);
};