#include "Engine/Network/TCPServer.hpp"
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Core/EngineCommon.hpp"

TCPServer::TCPServer( int port )
	: m_listenPort( port )
	, m_listenSocket( INVALID_SOCKET )
	, m_timeval{ 01, 01 }
{
	FD_ZERO( &m_listenSet );
}

TCPServer::~TCPServer()
{
	FD_ZERO( &m_listenSet );
	m_listenSocket = INVALID_SOCKET;
	m_timeval = timeval{ 01, 01 };
}

void TCPServer::Bind()
{
	struct addrinfo addrHintsIn;
	struct addrinfo* pAddrOut;

	ZeroMemory( &addrHintsIn, sizeof( addrHintsIn ) );
	addrHintsIn.ai_family = AF_INET;
	addrHintsIn.ai_socktype = SOCK_STREAM;
	addrHintsIn.ai_protocol = IPPROTO_TCP;
	addrHintsIn.ai_flags = AI_PASSIVE;

	std::string serverPort( std::to_string( m_listenPort ) );
	int iResult = getaddrinfo( NULL, serverPort.c_str(), &addrHintsIn, &pAddrOut );
	if( iResult != 0 )
	{
		g_theConsole->Error( "Call to getaddrinfo() failed, error = %i", iResult );
	}

	// Create a socket to listen for connections
	m_listenSocket = socket( pAddrOut->ai_family, pAddrOut->ai_socktype, pAddrOut->ai_protocol );
	if( m_listenSocket == INVALID_SOCKET )
	{
		g_theConsole->Error( "Listen socket creation error, error = %i", WSAGetLastError() );
	}

	unsigned long blockingMode = 1;
	iResult = ioctlsocket( m_listenSocket, FIONBIO, &blockingMode );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "Call to ioctlsocket error, error = %i", WSAGetLastError() );
	}

	// Bind the listening socket
	iResult = bind( m_listenSocket, pAddrOut->ai_addr, (int)pAddrOut->ai_addrlen );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "Sever socket bind failed, error = %i", WSAGetLastError() );
	}
}

void TCPServer::Listen()
{
	// Start listening for client connections
	int iResult = listen( m_listenSocket, SOMAXCONN );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "Call to listen failed, error = %i", WSAGetLastError() );
	}
}

void TCPServer::StopListen()
{
	if( m_listenSocket != INVALID_SOCKET )
	{
		int iResult = closesocket( m_listenSocket );
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->Error( "Call to closesocket on server listen socket failed, error = %i", WSAGetLastError() );
		}
		m_listenSocket = INVALID_SOCKET;
	}
}

TCPSocket TCPServer::Accept()
{
	//TCPSocket clientSocket;
	SOCKET clientSocket = INVALID_SOCKET;

	FD_ZERO( &m_listenSet );
	FD_SET( m_listenSocket, &m_listenSet );
	int iResult = select( 0, &m_listenSet, NULL, NULL, &m_timeval );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "Call to select failed %i", WSAGetLastError() );
	}
	if( FD_ISSET( m_listenSocket, &m_listenSet ) )
	{
		clientSocket = accept( m_listenSocket, NULL, NULL );

		if( clientSocket == INVALID_SOCKET )
		{
			g_theConsole->Error( "Call to accept failed %i", WSAGetLastError() );
		}
		
	}

	return TCPSocket( clientSocket );
}
