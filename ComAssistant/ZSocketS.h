#pragma once
#include <string>
#include<winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define eErrorC(x) static_cast<int>(eErrorCodeNum::x)

enum class eErrorCodeNum
{
	eSuccess = 0,
	eNotConnected,
	eAddrOrPortError
};
class ZSocketS
{
public:
	ZSocketS();
	~ZSocketS();
	int Connect(std::string strIPAddr, int nPort);
	int IsConnected(int nMilliSecond);
private:
	SOCKET m_socket;
	/*
		WSAStartup错误码介绍
	
		WSASYSNOTREADY  网络通信中下层的网络子系统没准备好
		WSAVERNOTSUPPORTED  Socket 实现提供版本和socket需要的版本不符
		WSAEINPROGRESS  一个阻塞的Socket操作正在进行
		WSAEPROCLIM  Socket的实现超过Socket支持的任务数限制
		WSAEFAULT   lpWSAData参数不是一个合法的指针
	*/
	std::string m_strIPAddr;
	int m_nPort;
	WSADATA m_WSD;
	sockaddr_in m_addrServer;
	sockaddr_in m_addrClient;
	HANDLE m_evtIsConnected;
};
