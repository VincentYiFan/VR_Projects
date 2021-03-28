#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <array>
#include <limits>

static const std::size_t DATA_PENDING =
std::numeric_limits<std::size_t>::max();

TCPSocket::TCPSocket()
	: m_mode(Mode::Nonblocking)
	, m_socket(INVALID_SOCKET)
	, m_timeval{0l, 0l}
	, m_bufferSize(256)
	, m_receiveSize(0)
	, m_bufferPtr(NULL)
{
	FD_ZERO( &m_fdSet );
	m_bufferPtr = new char[m_bufferSize];
}

TCPSocket::TCPSocket( SOCKET socket, Mode mode, std::size_t bufferSize )
	: m_socket( socket )
	, m_mode( mode )
	, m_timeval{0l, 0l}
	, m_bufferSize( bufferSize )
	, m_receiveSize( 0 )
	, m_bufferPtr( NULL )
{
	FD_ZERO( &m_fdSet );
	m_bufferPtr = new char[m_bufferSize];
}

TCPSocket::TCPSocket( const TCPSocket& src )
	: m_mode( src.m_mode )
	, m_socket( src.m_socket )
	, m_fdSet( src.m_fdSet )
	, m_timeval( src.m_timeval )
	, m_bufferSize( src.m_bufferSize )
	, m_receiveSize( 0 )
	, m_bufferPtr( NULL )
{
	if( m_bufferSize > 0 )
	{
		m_bufferPtr = new char[m_bufferSize];
	}
}

TCPSocket::~TCPSocket()
{
	m_mode = Mode::Invalid;
	m_socket = INVALID_SOCKET;
	FD_ZERO( &m_fdSet );
	m_bufferSize = 0;
	m_receiveSize = 0;
	delete[] m_bufferPtr;
}

TCPSocket& TCPSocket::operator=( const TCPSocket& src )
{
	if( this != &src )
	{
		m_mode = src.m_mode;
		m_bufferSize = src.m_bufferSize;
		m_receiveSize = src.m_receiveSize;
		m_socket = src.m_socket;
		if( m_bufferSize > 0 )
		{
			m_bufferPtr = new char[m_bufferSize];
		}
	}
	return *this;
}

std::string TCPSocket::GetAddress()
{
	std::array<char, 64> addressStr;
	DWORD outLen = 64;

	sockaddr clientAddr;
	int addrSize = sizeof( clientAddr );
	int iResult = getpeername( m_socket, &clientAddr, &addrSize );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "ERROR: Call to Getpeername failed: %i", WSAGetLastError() );
	}

	iResult = WSAAddressToStringA( &clientAddr, addrSize, NULL, &addressStr[0], &outLen );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "ERROR: Call to WSAAddressToStringA failed: %i", WSAGetLastError() );
	}
	addressStr[outLen] = NULL;
	return std::string( &addressStr[0] );
}

void TCPSocket::Send( char const* dataPtr, std::size_t length )
{
	int iResult = send( m_socket, dataPtr, (int)length, 0 );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "Send on socket failed, error = %i", WSAGetLastError() );
		closesocket( m_socket );
	}
	else if( iResult < (int)length )
	{
		g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Connection socket creation error, error = %i", WSAGetLastError() ) );
		closesocket( m_socket );
	}
}

void TCPSocket::SendMsg( eMessageType id, char const* dataPtr, std::size_t length )
{
	switch( id )
	{
	case eMessageType::TEXT_MESSAGE:
	{
		char buffer[512];
		//MessageHeader* header = new MessageHeader( (uint16_t)id, (uint16_t)length );
		//TCPMessageData messageData = TCPMessageData( *header, std::string( dataPtr ) );
		
		MessageHeader* header = reinterpret_cast<MessageHeader*>(&buffer[0]);
		header->m_id = (uint16_t)id;
		header->m_size = (uint16_t)length;
		
		//memcpy( buffer, &messageData, sizeof( MessageHeader ) );

		std::string str( dataPtr );
		for( int i = 0; i < str.size(); ++i )
		{
			buffer[ i + 4 ] = str[i];
		}

		int l = (int)length + 4;
		Send( &buffer[0], l );
	}
	break;

	case eMessageType::CLIENT_DISCONNECTING:
	{
		// ToDo: Finish this
		// m_clientSocket.Close();
		// m_serverSocket.Close();
		//g_theConsole->Printf( "Client disconnected" );
	}
	break;
	}
}

TCPMessageData TCPSocket::Receive()
{
	std::array< char, 256 > buffer;
	int iResult = ::recv( m_socket, &buffer[0], static_cast<int>(buffer.size() - 1), 0 );

	//if( WSAGetLastError() == WSAEWOULDBLOCK ) { // currently no data available
	//	return messageData;
	//}

	if( iResult != SOCKET_ERROR )
	{
		MessageHeader* headerPtr = reinterpret_cast< MessageHeader* >( &buffer[0] );
		buffer[iResult] = NULL;
		return TCPMessageData( *headerPtr, &buffer[4] );
	}
	return TCPMessageData();
}

bool TCPSocket::IsDataAvailable()
{
	if( m_mode == Mode::Nonblocking )
	{
		FD_ZERO( &m_fdSet );
		FD_SET( m_socket, &m_fdSet );
		int iResult = select( 0, &m_fdSet, NULL, NULL, &m_timeval );
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->Error( "Select on socket in non-blocking mode failed, error = %i", WSAGetLastError() );
			closesocket( m_socket );
		}
		return FD_ISSET( m_socket, &m_fdSet );
	}
	else
	{
		g_theConsole->Error( "Function isDataAvailable is only valid in non-blocking mode" );
		return false;
	}
}

void TCPSocket::ShutDown()
{
	int iResult = ::shutdown( m_socket, SD_SEND );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "Call to shutdown failed, error = %i", WSAGetLastError() );
		closesocket( m_socket );
	}
}

void TCPSocket::Close()
{
	int iResult = closesocket( m_socket );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "Call to closesocket failed, error = %i", WSAGetLastError() );
	}
	m_socket = INVALID_SOCKET;
}
