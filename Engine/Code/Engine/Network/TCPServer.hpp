#pragma once
#pragma warning(disable: 6319)

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdint>
#include <string>

class TCPSocket;


class TCPServer
{
public:
	TCPServer( int port = 48000 );
	~TCPServer();

	void		Bind();
	void		Listen();
	void		StopListen();
	TCPSocket	Accept();

private:
	int			m_listenPort;
	FD_SET		m_listenSet;
	SOCKET		m_listenSocket;
public:
	timeval		m_timeval;
};






