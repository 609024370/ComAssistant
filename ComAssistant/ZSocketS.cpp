#include "pch.h"
#include "ZSocketS.h"

ZSocketS::ZSocketS()
{
	/*
		当一个应用程序调用WSAStartup函数时，操作系统根据请求的Socket版本来搜索相应的Socket库，然后绑定找到的Socket库到该应用程序中。以后应用程序就可以调用所请求的Socket库中的其它Socket函数了。该函数执行成功后返回0。
	例：假如一个程序要使用2.2版本的Socket,那么程序代码如下
	*/
	//WSAStartup(MAKEWORD(2, 2), &wsd);    //初始化套接字

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
	WSACleanup();     //用于释放ws2_32.dll动态链接库初始化时分配的资源
}

int ZSocketS::Connect(std::string strIPAddr, int nPort)
{
	/*
		socket 的定义如下
		SOCKET WSAAPI socket(
		_In_ int af,
		_In_ int type,
		_In_ int protocol
		);

		建立一个socket用于连接
		af:address family，如AF_INET
		type:连接类型，通常是SOCK_STREAM或SOCK_DGRAM
		protocol:协议类型，通常是IPPROTO_TCP或IPPROTO_UDP
		返回值：socket的编号，为-1表示失败
	*/
	m_socket = socket(AF_INET, SOCK_STREAM, 0);

	m_strIPAddr = strIPAddr;
	m_nPort = nPort;
	if (!m_strIPAddr.length() || m_nPort == -1)
	{
		return eErrorC(eAddrOrPortError);
	}

	m_addrServer.sin_family = AF_INET;             //设置服务器地址家族
	m_addrServer.sin_port = htons(m_nPort);
	inet_pton(AF_INET, m_strIPAddr.c_str(), &m_addrServer.sin_addr);

	/*
		bind的语法:int bind( int sockfd, struct sockaddr* addr, socklen_t addrlen)
		返回：0──成功， -1──失败

		参数sockfd
		指定地址与哪个套接字绑定，这是一个由之前的socket函数调用返回的套接字。调用bind的函数之后，该套接字与一个相应的地址关联，发送到这个地址的数据可以通过这个套接字来读取与使用

		参数addr
		指定地址。这是一个地址结构，并且是一个已经经过填写的有效的地址结构。调用bind之后这个地址与参数sockfd指定的套接字关联，从而实现上面所说的效果

		参数addrlen
		正如大多数socket接口一样，内核不关心地址结构，当它复制或传递地址给驱动的时候，它依据这个值来确定需要复制多少数据。	这已经成为socket接口中最常见的参数之一了

	*/
	int nRet = bind(m_socket, (sockaddr*)&m_addrServer, sizeof(m_addrServer));    //把名字和套接字绑定
	if (nRet == SOCKET_ERROR)
	{
		TRACE("套接字绑定失败\r\n");
		closesocket(m_socket);
		WSACleanup();
		return eErrorC(eNotConnected);
	}

	/*
		listen 函数在一般在调用bind之后-调用accept之前调用，语法如下：
		int listen(int sockfd, int backlog)
		返回：0──成功， -1──失败

		参数sockfd
		被listen函数作用的套接字，sockfd之前由socket函数返回。在被socket函数返回的套接字fd之时，它是一个主动连接的套接字，也就是此时系统假设用户会对这个套接字调用connect函数，
	期待它主动与其它进程连接，然后在服务器编程中，用户希望这个套接字可以接受外来的连接请求，也就是被动等待用户来连接。由于系统默认时认为一个套接字是主动连接的，
	所以需要通过某种方式来告诉系统，用户进程通过系统调用listen来完成这件事

		参数backlog
		这个参数涉及到一些网络的细节。在进程正理一个一个连接请求的时候，可能还存在其它的连接请求。因为TCP连接是一个过程，所以可能存在一种半连接的状态，
	有时由于同时尝试连接的用户过多，使得服务器进程无法快速地完成连接请求。如果这个情况出现了，服务器进程希望内核如何处理呢？
	内核会在自己的进程空间里维护一个队列以跟踪这些完成的连接但服务器进程还没有接手处理或正在进行的连接，这样的一个队列内核不可能让其任意大，所以必须有一个大小的上限。这个backlog告诉内核使用这个数值作为上限。
	服务器进程不能随便指定一个数值，内核有一个许可的范围。这个范围是实现相关的。很难有某种统一，一般这个值会小30以内
	*/
	int iLisRet = listen(m_socket, 0);    //进行监听
	if (nRet == SOCKET_ERROR)
	{
		TRACE("套接字监听失败\r\n");
		closesocket(m_socket);
		WSACleanup();
		return eErrorC(eNotConnected);
	}

	/*
		accept 语法如下:
		SOCKET accept(SOCKET sock, struct sockaddr *addr, int *addrlen);

		它的参数与 listen() 和 connect() 是相同的：sock 为服务器端套接字，addr 为 sockaddr_in 结构体变量，
	addrlen 为参数 addr 的长度，可由 sizeof() 求得
		accept() 返回一个新的套接字来和客户端通信，addr 保存了客户端的IP地址和端口号，而 sock 是服务器端的套接字，
	大家注意区分。后面和客户端通信时，要使用这个新生成的套接字，而不是原来服务器端的套接字。
		accept()系统调用主要用在基于连接的套接字类型，比如SOCK_STREAM和SOCK_SEQPACKET。它提取出所监听套接字的等待连接队列中第一个连接请求，
	创建一个新的套接字，并返回指向该套接字的文件描述符。新建立的套接字不在监听状态，原来所监听的套接字也不受该系统调用的影响
	备注：新建立的套接字准备发送send()和接收数据recv()
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