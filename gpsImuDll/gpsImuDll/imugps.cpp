#include "stdafx.h"
#include "imugps.h"

imugps::imugps() {}


imugps::~imugps() {
	if (imugps::running == true) {
		imugps::running = false;
		if (imugps::axioThread.joinable()) {
			imugps::axioThread.join();
			std::cout << "thread stopped" << std::endl;
		}
	}
	closesocket(imugps::axioSock);
}


imugps::imugps(const std::string& ip, int port) {
	imugps::IMU_IP_ADDR = ip;
	imugps::IMU_PORT = port;
}


imugps::imugps(const std::string& ip, int port, int timeout) {
	imugps::IMU_IP_ADDR = ip;
	imugps::IMU_PORT = port;
	imugps::TIMEOUT = timeout;
}


int imugps::connect() {
	imugps::axioSock = makeSock(imugps::IMU_IP_ADDR, imugps::IMU_PORT, imugps::TIMEOUT);
	if (imugps::axioSock != 0) {
		return 1;
	}
	else {
		return 0;
	}
}


void imugps::startLog(const std::string& filename) {
	if (imugps::running == false) {
		imugps::running = true;
		imugps::axioThread = std::thread(&imugps::axioToThread, this, filename);
		std::cout << "Axio Log started" << std::endl;
	}
	else {
		std::cout << "Axio Log already running" << std::endl;
	}
}


void imugps::stopLog() {
	if (imugps::running == true) {
		imugps::running = false;
		if (imugps::axioThread.joinable()) {
			imugps::axioThread.join();
			std::cout << "Axio Log stopped" << std::endl;
		}
	}
}


//Private funcs----------------------------------------------------------------------
void imugps::axioToThread(const std::string& filename) {
	int64_t currtime;
	std::string axioLine;
	std::string CANid;
	std::string CANdata;
	std::string ext = ".txt";
	int IMUtag;
	int GPStag;

	//Open IMU files:
	std::ofstream IMUlogfile, GPSlogfile;
	IMUlogfile.open(filename + "_IMU" + ext);
	GPSlogfile.open(filename +  "_GPS" + ext);

	char axioMESSAGE;
	memset(&axioMESSAGE, 0, sizeof(axioMESSAGE));

	while (running) {
		int  i = 0;
		int bytesRecv = 0;
		int MESSAGElen = 1024;
		axioLine = "";
		while (i < MESSAGElen) {
			bytesRecv = recv(imugps::axioSock, &axioMESSAGE, sizeof(axioMESSAGE), NULL);
			i += bytesRecv;
			axioLine += axioMESSAGE;
			if ((i == 10) & (axioLine.size() < 11)) {
				MESSAGElen = (axioMESSAGE & 0xff) + 1;
				i = 0;
			}
			if (bytesRecv < 1) {
				std::cout << "bytesRecv == 0" << std::endl;
				break;
			}
		}
		currtime = gettime();
		axioLine = axioLine.substr(axioLine.size() - 12, 12);
		CANid = toHex(axioLine.substr(0, 4), true);
		CANdata = axioLine.substr(4, 8);

		IMUtag = imugps::isIMU(CANid);
		GPStag = imugps::isGPS(CANid);
		if (IMUtag != -1) {
			std::vector<long double> result(imugps::imuDecode(CANdata, IMUtag));
			IMUlogfile << currtime << " " << CANid.substr(4, 2) + CANid.substr(2, 2) << " ";
			for (unsigned int j = 0; j < result.size(); j++) {
				int decNum = abs(result.at(j));
				long decimal = 10000000*(abs(result.at(j)) - (decNum));
				if (result.at(j) < 0) {
					IMUlogfile << "-";
				}
				IMUlogfile << decNum<< "."<<decimal;
				if (j != (result.size() - 1)) {
					IMUlogfile << " ";
				}
			}
			IMUlogfile << "\n";
		}
		else if (GPStag != -1) {
			std::vector<long double> result(imugps::gpsDecode(CANdata, GPStag));
			GPSlogfile << currtime << " " << CANid.substr(4, 2) + CANid.substr(2, 2) << " ";
			for (unsigned int j = 0; j < result.size(); j++) {
				int decNum = abs(result.at(j));
				long decimal = 10000000 * (abs(result.at(j)) - (decNum));
				if (result.at(j) < 0) {
					GPSlogfile << "-";
				}
				GPSlogfile << decNum<<"."<<decimal;
				if (j != (result.size() - 1)) {
					GPSlogfile << " ";
				}
			}
			GPSlogfile << "\n";
		}
	}
	IMUlogfile.close();
	GPSlogfile.close();
}


int imugps::isGPS(const std::string& CANid) {
	if (CANid == "1CF3FE0C") { //gps data
		return 1;
	}
	else if (CANid == "1CE8FE0C") {// heading, ground speed, pitch 
		return 2;
	}
	else {
		return -1;
	}
}


std::vector<long double> imugps::gpsDecode(const std::string& CANdata, int GPStag) {
	std::vector<long double> ret;
	//(value*factor)+offset
	long double gpsFactor = (1.0 / 10000000), gpsOffset = -210;
	long double factor1 = (1.0 / 128), factor2 = (1.0 / 256), offset1 = -200;
	if ((GPStag < 1) | (GPStag > 2)) {
		ret.push_back(-1);
		return ret;
	}

	switch (GPStag) {
	case 1://gps, long, lat
		ret.push_back(gpsFactor*(bytestoint(CANdata.substr(4, 4), true)) + gpsOffset);//long
		ret.push_back(gpsFactor*(bytestoint(CANdata.substr(0, 4), true)) + gpsOffset);//lat
		break;
	case 2://heading, ground speed, pitch
		ret.push_back(factor1*(bytestoint(CANdata.substr(0, 2), true)));//heading
		ret.push_back(factor2*(bytestoint(CANdata.substr(2, 2), true)));//ground speed
		ret.push_back(factor1 *(bytestoint(CANdata.substr(5, 2), true)) + offset1);//pitch
		break;
	default:
		break;
	}

	return ret;
}


int imugps::isIMU(const std::string& CANid) {
	if (CANid == "5C95FF0C") {//Yaw rate, lateral accel, temperature
		return 1;
	}
	else if (CANid == "5C96FF0C") {//Roll rate, longitudinal accel, error msg
		return 2;
	}
	else if (CANid == "5C97FF0C") {//Pitch rate, vertical accel
		return 3;
	}
	else if (CANid == "5C98FF0C") {//Roll incline, Pitch incline 
		return 4;
	}
	else {
		return -1;
	}
}


std::vector<long double> imugps::imuDecode(const std::string& CANdata, int IMUtag) {
	std::vector<long double> ret;
	//(value-offset)*factor
	long double degfactor = .0078125, offset = 32767, accfactor = 0.01, incfactor = 0.002;
	if ((IMUtag < 1) | (IMUtag > 4)) {
		ret.push_back(-1);
		return ret;
	}

	switch (IMUtag) {
	case 1:
		ret.push_back(accfactor * (bytestoint(CANdata.substr(2, 2), false) - offset));
		ret.push_back(degfactor * (bytestoint(CANdata.substr(4, 2), false) - offset));
		ret.push_back(((int)CANdata[0]) & 0xff);
		ret.push_back((((int)CANdata[1]) & 0xC0) >> 6);
		ret.push_back((((int)CANdata[1]) & 0x30) >> 4);
		ret.push_back((((int)CANdata[1]) & 0x0C) >> 2);
		ret.push_back((int)CANdata[6] & 0xff);
		break;
	case 2:
		ret.push_back(accfactor * (bytestoint(CANdata.substr(2, 2), false) - offset));
		ret.push_back(degfactor * (bytestoint(CANdata.substr(4, 2), false) - offset));
		ret.push_back((int)CANdata[0] & 0xff);
		ret.push_back((((int)CANdata[1]) & 0xC0) >> 6);
		ret.push_back((((int)CANdata[1]) & 0x30) >> 4);
		ret.push_back(bytestoint(CANdata.substr(6, 2), false));
		break;
	case 3:
		ret.push_back(accfactor * (bytestoint(CANdata.substr(2, 2), false) - offset));
		ret.push_back(degfactor * (bytestoint(CANdata.substr(4, 2), false) - offset));
		ret.push_back((int)CANdata[0] & 0xff);
		ret.push_back((((int)CANdata[1]) & 0xC0) >> 6);
		ret.push_back((((int)CANdata[1]) & 0x30) >> 4);
		break;
	case 4:
		ret.push_back(incfactor * (bytestoint(CANdata.substr(2, 2), false) - offset));
		ret.push_back(incfactor * (bytestoint(CANdata.substr(4, 2), false) - offset));
		ret.push_back((int)CANdata[0] & 0xff);
		ret.push_back((((int)CANdata[1]) & 0xC0) >> 6);
		ret.push_back((((int)CANdata[1]) & 0x30) >> 4);
		break;
	default:
		break;
	}
	return ret;
}