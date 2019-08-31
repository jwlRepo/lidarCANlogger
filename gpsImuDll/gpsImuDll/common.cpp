#include "common.h"
#include "stdafx.h"

unsigned long int bytestoint(const std::string& s, bool flip) {
	double offset = 0;
	if (true == flip) {
		offset = s.size() - 1;
	}

	unsigned long int res = (s[offset] & 0xff);
	for (unsigned int i = 1; i < s.size(); i++) {
		res = (res << 8) | ((unsigned long int)s[abs(offset - i)] & 0xff);
	}
	return res;
}

std::string toHex(const std::string& s, bool upper_case)
{
	std::ostringstream ret;
	for (std::string::size_type i = 0; i < s.length(); ++i)
	{
		int z = s[i] & 0xff;
		ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << z;
	}

	return ret.str();
}

int64_t gettime() {
	//get time stamp in milliseconds
	int64_t a = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return a;
}

SOCKET makeSock(std::string& ip, int port, int timeout) {
	WSADATA WinSockData;
	WORD DLLVersion;
	DLLVersion = MAKEWORD(2, 1);
	std::wstring stemp = s2ws(ip);
	PCWSTR formatted_ip = stemp.c_str();

	//starting up the socket stuff, shutfown if it fails
	if (WSAStartup(DLLVersion, &WinSockData) != 0) {
		std::cout << "Socket startup failed" << std::endl;
		return 0;
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	InetPton(AF_INET, formatted_ip, &addr.sin_addr.s_addr);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, NULL);

	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	if (connect(sock, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Could not connect to IP: " << ip << " Port: "<< port<< std::endl;
		return 0;
	}

	return sock;
}

std::wstring s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}