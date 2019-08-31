#pragma once
#include "lidar.h"
#include "imugps.h"

#ifdef __cplusplus
extern "C" {
	#endif

	//Lidar ---------------------------------------------------------
	extern DllExport lidar* createLidar(const char*, int, int);
	extern DllExport int lidar_conn(lidar* a_pObject);
	extern DllExport void lidar_startLog(lidar* a_pObject, const char *str);
	extern DllExport void lidar_stopLog(lidar* a_pObject);
	extern DllExport void lidar_setPlane(lidar* a_pObject, int);
	extern DllExport void lidar_del(lidar* a_pObject);

	//Axiomatic ------------------------------------------------------
	extern DllExport imugps* createAxio(const char*, int, int);
	extern DllExport int axio_conn(imugps* a_pObject);
	extern DllExport void axio_startLog(imugps* a_pObject, const char *str);
	extern DllExport void axio_stopLog(imugps* a_pObject);
	extern DllExport void axio_del(imugps* a_pObject);

#ifdef __cplusplus
}
#endif