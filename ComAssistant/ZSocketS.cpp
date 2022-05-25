#include "pch.h"
#include "ZSocketS.h"

ZSocketS::ZSocketS()
{
	/*
		��һ��Ӧ�ó������WSAStartup����ʱ������ϵͳ���������Socket�汾��������Ӧ��Socket�⣬Ȼ����ҵ���Socket�⵽��Ӧ�ó����С��Ժ�Ӧ�ó���Ϳ��Ե����������Socket���е�����Socket�����ˡ��ú���ִ�гɹ��󷵻�0��
	��������һ������Ҫʹ��2.2�汾��Socket,��ô�����������
	*/
	//WSAStartup(MAKEWORD(2, 2), &wsd);    //��ʼ���׽���

	if (WSAStartup(MAKEWORD(2, 2), &m_WSD))
	{
		printf("Initlalization Error!");
		//return -1;
	}
	m_evtIsConnected = CreateEvent(NULL, TRUE, FALSE, NULL);
	ResetEvent(m_evtIsConnected);
	m_strIPAddr.empty();
	m_nPort = -1;
}

ZSocketS::~ZSocketS()
{
	WSACleanup();     //�����ͷ�ws2_32.dll��̬���ӿ��ʼ��ʱ�������Դ
}

int ZSocketS::Connect(std::string strIPAddr, int nPort)
{
	/*
		socket �Ķ�������
		SOCKET WSAAPI socket(
		_In_ int af,
		_In_ int type,
		_In_ int protocol
		);

		����һ��socket��������
		af:address family����AF_INET
		type:�������ͣ�ͨ����SOCK_STREAM��SOCK_DGRAM
		protocol:Э�����ͣ�ͨ����IPPROTO_TCP��IPPROTO_UDP
		����ֵ��socket�ı�ţ�Ϊ-1��ʾʧ��
	*/
	m_socket = socket(AF_INET, SOCK_STREAM, 0);

	m_strIPAddr = strIPAddr;
	m_nPort = nPort;
	if (!m_strIPAddr.length() || m_nPort == -1)
	{
		return eErrorC(eAddrOrPortError);
	}

	m_addrServer.sin_family = AF_INET;             //���÷�������ַ����
	m_addrServer.sin_port = htons(m_nPort);
	inet_pton(AF_INET, m_strIPAddr.c_str(), &m_addrServer.sin_addr);

	/*
		bind���﷨:int bind( int sockfd, struct sockaddr* addr, socklen_t addrlen)
		���أ�0�����ɹ��� -1����ʧ��

		����sockfd
		ָ����ַ���ĸ��׽��ְ󶨣�����һ����֮ǰ��socket�������÷��ص��׽��֡�����bind�ĺ���֮�󣬸��׽�����һ����Ӧ�ĵ�ַ���������͵������ַ�����ݿ���ͨ������׽�������ȡ��ʹ��

		����addr
		ָ����ַ������һ����ַ�ṹ��������һ���Ѿ�������д����Ч�ĵ�ַ�ṹ������bind֮�������ַ�����sockfdָ�����׽��ֹ������Ӷ�ʵ��������˵��Ч��

		����addrlen
		��������socket�ӿ�һ�����ں˲����ĵ�ַ�ṹ���������ƻ򴫵ݵ�ַ��������ʱ�����������ֵ��ȷ����Ҫ���ƶ������ݡ�	���Ѿ���Ϊsocket�ӿ�������Ĳ���֮һ��

	*/
	int nRet = bind(m_socket, (sockaddr*)&m_addrServer, sizeof(m_addrServer));    //�����ֺ��׽��ְ�
	if (nRet == SOCKET_ERROR)
	{
		TRACE("�׽��ְ�ʧ��\r\n");
		closesocket(m_socket);
		WSACleanup();
		return eErrorC(eNotConnected);
	}

	/*
		listen ������һ���ڵ���bind֮��-����accept֮ǰ���ã��﷨���£�
		int listen(int sockfd, int backlog)
		���أ�0�����ɹ��� -1����ʧ��

		����sockfd
		��listen�������õ��׽��֣�sockfd֮ǰ��socket�������ء��ڱ�socket�������ص��׽���fd֮ʱ������һ���������ӵ��׽��֣�Ҳ���Ǵ�ʱϵͳ�����û��������׽��ֵ���connect������
	�ڴ��������������������ӣ�Ȼ���ڷ���������У��û�ϣ������׽��ֿ��Խ�����������������Ҳ���Ǳ����ȴ��û������ӡ�����ϵͳĬ��ʱ��Ϊһ���׽������������ӵģ�
	������Ҫͨ��ĳ�ַ�ʽ������ϵͳ���û�����ͨ��ϵͳ����listen����������

		����backlog
		��������漰��һЩ�����ϸ�ڡ��ڽ�������һ��һ�����������ʱ�򣬿��ܻ���������������������ΪTCP������һ�����̣����Կ��ܴ���һ�ְ����ӵ�״̬��
	��ʱ����ͬʱ�������ӵ��û����࣬ʹ�÷����������޷����ٵ��������������������������ˣ�����������ϣ���ں���δ����أ�
	�ں˻����Լ��Ľ��̿ռ���ά��һ�������Ը�����Щ��ɵ����ӵ����������̻�û�н��ִ�������ڽ��е����ӣ�������һ�������ں˲�����������������Ա�����һ����С�����ޡ����backlog�����ں�ʹ�������ֵ��Ϊ���ޡ�
	���������̲������ָ��һ����ֵ���ں���һ����ɵķ�Χ�������Χ��ʵ����صġ�������ĳ��ͳһ��һ�����ֵ��С30����
	*/
	int iLisRet = listen(m_socket, 0);    //���м���
	if (nRet == SOCKET_ERROR)
	{
		TRACE("�׽��ּ���ʧ��\r\n");
		closesocket(m_socket);
		WSACleanup();
		return eErrorC(eNotConnected);
	}

	/*
		accept �﷨����:
		SOCKET accept(SOCKET sock, struct sockaddr *addr, int *addrlen);

		���Ĳ����� listen() �� connect() ����ͬ�ģ�sock Ϊ���������׽��֣�addr Ϊ sockaddr_in �ṹ�������
	addrlen Ϊ���� addr �ĳ��ȣ����� sizeof() ���
		accept() ����һ���µ��׽������Ϳͻ���ͨ�ţ�addr �����˿ͻ��˵�IP��ַ�Ͷ˿ںţ��� sock �Ƿ������˵��׽��֣�
	���ע�����֡�����Ϳͻ���ͨ��ʱ��Ҫʹ����������ɵ��׽��֣�������ԭ���������˵��׽��֡�
		accept()ϵͳ������Ҫ���ڻ������ӵ��׽������ͣ�����SOCK_STREAM��SOCK_SEQPACKET������ȡ���������׽��ֵĵȴ����Ӷ����е�һ����������
	����һ���µ��׽��֣�������ָ����׽��ֵ��ļ����������½������׽��ֲ��ڼ���״̬��ԭ�����������׽���Ҳ���ܸ�ϵͳ���õ�Ӱ��
	��ע���½������׽���׼������send()�ͽ�������recv()
	*/
	int nLen = sizeof(m_addrClient);
	SOCKET socketClient = accept(m_socket, (sockaddr*)&m_addrClient, &nLen);
	if (socketClient != INVALID_SOCKET)
	{
		//int getsockname(int sockfd, struct sockaddr *localaddr, socklen_t *addrlen)

		if (getsockname(socketClient, (struct sockaddr*)&m_addrClient, &nLen) != -1)
		{
			char chAddr[20] = { 0 };
			inet_ntop(AF_INET, &m_addrClient.sin_addr, chAddr, sizeof(chAddr));
			printf("listen address = %s:%d\n", chAddr, ntohs(m_addrClient.sin_port));
		}
		else
		{
			printf("getsockname error\n");
			//exit(0);
		}
	}

	return NO_ERROR;
}

int ZSocketS::IsConnected(int nMilliSecond)
{
	MSG msg;
	int nCount = 0;
	nMilliSecond /= 10;
	DWORD wRet = WAIT_FAILED;
	while (nCount++ < nMilliSecond)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		wRet = WaitForSingleObject(m_evtIsConnected, 10);

		Sleep(10);
		if (wRet == WAIT_OBJECT_0)
		{
			return NO_ERROR;
		}
	}
	return eErrorC(eNotConnected);
}