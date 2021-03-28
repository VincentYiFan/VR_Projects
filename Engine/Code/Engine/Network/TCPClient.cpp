#include "Engine/Network/TCPClient.hpp"
#include "Engine/Core/EngineCommon.hpp"

TCPSocket TCPClient::Connect( std::string const& host, std::uint16_t port, Mode mode )
{
	m_mode = mode;

	// Resolve the port locally
	struct addrinfo  addrHintsIn;
	struct addrinfo* pAddrOut = NULL;

	ZeroMemory( &addrHintsIn, sizeof( addrHintsIn ) );
	addrHintsIn.ai_family = AF_INET;
	addrHintsIn.ai_socktype = SOCK_STREAM;
	addrHintsIn.ai_protocol = IPPROTO_TCP;

	// Get the Server IP address and port
	std::string serverPort( std::to_string( port ) );
	int iResult = ::getaddrinfo( host.c_str(), serverPort.c_str(), &addrHintsIn, &pAddrOut );
	if( iResult != 0 )
	{
		g_theConsole->Error( "Call to getaddrinfo() failed, error = %i", iResult );
	}

	// Create the connection socket.
	SOCKET socket = ::socket( pAddrOut->ai_family, pAddrOut->ai_socktype, pAddrOut->ai_protocol );
	if( socket == INVALID_SOCKET )
	{
		g_theConsole->Error( "Connection socket creation error, error = %i", WSAGetLastError() );
		freeaddrinfo( pAddrOut );
	}

	// Connect to the server
	iResult = ::connect( socket, pAddrOut->ai_addr, (int)pAddrOut->ai_addrlen );
	if( iResult == SOCKET_ERROR )
	{
		closesocket( socket );
		socket = INVALID_SOCKET;
	}
	freeaddrinfo( pAddrOut );

	if( socket == INVALID_SOCKET )
	{
		g_theConsole->Error( "Connection failed" );
	}

	// Set blocking mode as needed.
	if( m_mode == Mode::Nonblocking )
	{
		u_long winsockmode = 1;
		iResult = ioctlsocket( socket, FIONBIO, &winsockmode );
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->Error( "IOCTL on server listen socket failed, error = %i", WSAGetLastError() );
			closesocket( socket );
		}
	}
	return TCPSocket( socket, m_mode );
}
