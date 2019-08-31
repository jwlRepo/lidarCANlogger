#include "stdafx.h"
#include "caller.h"

//Lidar methods ---------------------------------------------------------
lidar* createLidar(const char *str, int port, int timeout) {
	std::string ip(str);
	return new lidar(ip, port, timeout);
}


int lidar_conn(lidar* a_pObject) {
	if (a_pObject != NULL) {
		return a_pObject->connect();
	}
}


void lidar_startLog(lidar* a_pObject, const char *str) {
	if (a_pObject != NULL) {
		std::string filename(str);
		a_pObject->startLog(filename);
	}
}


void lidar_stopLog(lidar* a_pObject) {
	if (a_pObject != NULL) {
		a_pObject->stopLog();
	}
}


void lidar_setPlane(lidar* a_pObject, int plane) {
	if (a_pObject != NULL) {
		a_pObject-> setPlane(plane);
	}
}


void lidar_del(lidar* a_pObject) {
	if (a_pObject != NULL) {
		delete a_pObject;
	}
}


//Axiomatic methods--------------------------------------------------------
imugps* createAxio(const char *str, int port, int timeout) {
	std::string ip(str);
	return new imugps(ip, port, timeout);
}


int axio_conn(imugps* a_pObject) {
	if (a_pObject != NULL) {
		return a_pObject->connect();
	}
}


void axio_startLog(imugps* a_pObject, const char *str) {
	if (a_pObject != NULL) {
		std::string filename(str);
		a_pObject->startLog(filename);
	}
}


void axio_stopLog(imugps* a_pObject) {
	if (a_pObject != NULL) {
		a_pObject->stopLog();
	}
}


void axio_del(imugps* a_pObject) {
	if (a_pObject != NULL) {
		delete a_pObject;
	}
}