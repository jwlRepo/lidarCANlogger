#include "stdafx.h"
#include "lidar.h"


lidar::lidar() {

}


lidar::~lidar() {
	if (lidar::running == true) {
		lidar::running = false;
		if (lidar::lidarThread.joinable()) {
			lidar::lidarThread.join();
			std::cout << "thread stopped" << std::endl;
		}
	}
	closesocket(lidar::lidarSock);
}


lidar::lidar(const std::string& ip, int port) {
	lidar::LIDAR_IP_ADDR = ip;
	lidar::LIDAR_PORT = port;
}


lidar::lidar(const std::string& ip, int port, int timeout) {
	lidar::LIDAR_IP_ADDR = ip;
	lidar::LIDAR_PORT = port;
	lidar::TIMEOUT = timeout;
}


int lidar::connect() {
	lidar::lidarSock=makeSock(lidar::LIDAR_IP_ADDR, lidar::LIDAR_PORT, lidar::TIMEOUT);
	if (lidar::lidarSock!=0) {
		return 1;
	}
	else {
		return 0;
	}
}


void lidar::startLog(const std::string& filename) {
	if (lidar::running == false) {
		lidar::running = true;
		lidar::lidarThread = std::thread(&lidar::lidarToThread, this, filename);
		std::cout << "Lidar Log Started" << std::endl;
	}
	else {
		std::cout << "Lidar Log already running" << std::endl;
	}
	
}


void lidar::stopLog() {
	if (lidar::running == true) {
		lidar::running = false;
		if (lidar::lidarThread.joinable()) {
			lidar::lidarThread.join();
			std::cout << "Lidar Log stopped" << std::endl;
		}
	}
}


void lidar::setPlane(int planes) {
	lidar::planes = planes;
}


//Private func------------------------------------------------------------------------
void lidar::lidarToThread(const std::string& filename) {
	const char* last_scan;
	std::string last_scan_string = "";
	last_scan_string += (char)2;
	last_scan_string += "sRN";
	last_scan_string += (char)32;
	last_scan_string += "LMDscandata";
	last_scan_string += (char)3;
	last_scan = last_scan_string.c_str();

	bool p1, p2, p3, p4;
	p1 = false; p3 = false; p3 = false; p4 = false; //keep track if plane has been recorded
	int p1Sel, p2Sel, p3Sel, p4Sel;
	p1Sel = 0; p2Sel = 0; p3Sel = 0; p4Sel = 0;//keep track if plane was selected

	std::string ext = ".txt";
	std::string prevtime;
	int64_t currtime;
	std::string lidarLine;
	char lidarMESSAGE;
	memset(&lidarMESSAGE, 0, sizeof(lidarMESSAGE));

	//Open lidar files:
	lidar::lidarsettingfile.open(filename + ext, std::ios_base::app);
	lidar::lidarlogfile.open(filename + "_lidar" + ext);

	send(lidar::lidarSock, last_scan, (int)strlen(last_scan), 0);
	do {
		//receive the data to the message array and set flag
		recv(lidar::lidarSock, &lidarMESSAGE, 1, NULL);
		lidarLine += lidarMESSAGE;
	} while (lidarMESSAGE != lidar::endChar);

	std::istringstream iss(lidarLine);
	std::vector<std::string> results((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	ptrdiff_t idx = std::find(results.begin(), results.end(), "DIST1") - results.begin();
	lidar::length = std::stoi(results.at(idx + 5), nullptr, 16);
	lidar::lidarsettingfile << "<Epoch Time Thread Started (ms)> " << gettime() << "\n";
	lidar::lidarsettingfile << "<Points per scan>" << lidar::length << "\n";
	lidar::lidarsettingfile << "<Start Angle>" << std::stoi(results.at(idx + 3), nullptr, 16) / 10000.0 << "\n";
	lidar::lidarsettingfile << "<Angle Size>" << std::stoi(results.at(idx + 4), nullptr, 16) / 10000.0 << "\n";
	lidar::lidarsettingfile.close();

	while (lidar::running==true) {
		send(lidar::lidarSock, last_scan, (int)strlen(last_scan), 0);
		lidarLine = "";
		do {
			recv(lidar::lidarSock, &lidarMESSAGE, 1, NULL);
			lidarLine += lidarMESSAGE;
		} while (lidarMESSAGE != lidar::endChar);

		currtime = gettime();
		std::istringstream iss(lidarLine);
		std::vector<std::string> results((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

		//Plane 1 = FA | Plane 2 = 0 | Plane 3 = FF06 | Plane 4 = FE0C
		p1Sel = lidar::planes / 8; 
		p2Sel = (lidar::planes / 4) % 2;
		p3Sel = (lidar::planes / 2) % 2;
		p4Sel = lidar::planes % 2;
		if (results.at(9) != prevtime){
			p1 = false; p2 = false; p3 = false; p4 = false;
		}

		if ((p1==false && p1Sel ==1 && results.at(15) == "FA") || (p2 == false && p2Sel == 1 && results.at(15) == "0") || (p3 == false && p3Sel == 1 && results.at(15) == "FF06") || (p4 == false && p4Sel == 1 && results.at(15) == "FE0C")) {

			lidar::lidarlogfile << currtime << " ";
			if (results.at(15) == "FA") {
				lidar::lidarlogfile << "P1" << " ";
				p1 = true;
			}
			else if (results.at(15) == "0") {
				lidar::lidarlogfile << "P2" << " ";
				p2 = true;
			}
			else if (results.at(15) == "FF06") {
				lidar::lidarlogfile << "P3" << " ";
				p3 = true;
			}
			else if (results.at(15) == "FE0C") {
				lidar::lidarlogfile << "P4" << " ";
				p4 = true;
			}
			else {
				std::cout << "unknown plane found" << std::endl;
			}

			ptrdiff_t idx = logSpecifics(results, 0, "DIST1");
			idx = logSpecifics(results, idx, "RSSI1");
			idx = logSpecifics(results, idx, "DIST2");
			idx = logSpecifics(results, idx, "RSSI2");
			idx = logSpecifics(results, idx, "DIST3");
			idx = logSpecifics(results, idx, "RSSI3");

			/*ptrdiff_t idx = std::find(results.begin(), results.end(), "DIST1") - results.begin();
			lidar::lidarlogfile << "DIST1 ";
			for (int data = idx + 6; data < (idx + lidar::length + 6); data++) {
				lidar::lidarlogfile << stoi(results.at(data), nullptr, 16);
				if (data != (idx + lidar::length + 5)) {
					lidar::lidarlogfile << " ";
				}
			}*/
			
			lidar::lidarlogfile << "\n";
		}
		prevtime = results.at(9);
	}
	lidar::lidarlogfile.close();
}

ptrdiff_t lidar::logSpecifics(std::vector<std::string>& arr, ptrdiff_t startDex, std::string label) {
	ptrdiff_t idx = std::find(arr.begin() + startDex, arr.end(), label) - arr.begin();
	if (idx != arr.size()) {
		lidar::lidarlogfile <<" "<<label << " ";
		for (ptrdiff_t data = idx + 6; data < (idx + lidar::length + 6); data++) {
			lidar::lidarlogfile << stoi(arr.at(data), nullptr, 16);
			if (data != (idx + lidar::length + 5)) {
				lidar::lidarlogfile << " ";
			}
		}
		return idx + lidar::length +6;
	}
	else if (startDex > 0) {
		idx = std::find(arr.begin(), arr.begin() + startDex, label) - arr.begin();
		if (idx < startDex) {
			lidar::lidarlogfile << " " << label << " ";
			for (ptrdiff_t data = idx + 6; data < (idx + lidar::length + 6); data++) {
				lidar::lidarlogfile << stoi(arr.at(data), nullptr, 16);
				if (data != (idx + lidar::length + 5)) {
					lidar::lidarlogfile << " ";
				}
			}
			return idx + lidar::length + 6;
		}
	}
	return startDex;
	
}