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
		WSAStartup���������
	
		WSASYSNOTREADY  ����ͨ�����²��������ϵͳû׼����
		WSAVERNOTSUPPORTED  Socket ʵ���ṩ�汾��socket��Ҫ�İ汾����
		WSAEINPROGRESS  һ��������Socket�������ڽ���
		WSAEPROCLIM  Socket��ʵ�ֳ���Socket֧�ֵ�����������
		WSAEFAULT   lpWSAData��������һ���Ϸ���ָ��
	*/
	std::string m_strIPAddr;
	int m_nPort;
	WSADATA m_WSD;
	sockaddr_in m_addrServer;
	sockaddr_in m_addrClient;
	HANDLE m_evtIsConnected;
};
