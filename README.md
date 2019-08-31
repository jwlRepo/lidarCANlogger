# lidarCANlogger
Half Semester work: 
Uses laptop to log CANbus data from GPS and CAN IMU and point cloud data from SICK MRS 1114 LiDAR for agriculture scanning with LiDAR testing.

Backend: gpsImuDll-C++ threaded ethernet listener for SICK MRS 1114 lidar and axiomatic CAN to ethernet converter for GPS. Final compilation as a .dll file for C# front end.

Frontend: Under repair! WpfApp1-utilizes C# WPF with data post marshalled from C++ to C#, unfortunately full working version was accidentally deleted, this is a previous versions with error still present. Will redo it when I have time. 

Executable: lidarAxioUI- Full program. UI allows for metadata, comment, log file location and LiDAR plane setting.
