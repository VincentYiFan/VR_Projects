#include "UDPSocket.hpp"
#include "Engine/Core/EngineCommon.hpp"

//#ifdef TEST_MODE
//	#define LOG_ERROR(...) printf(Stringf(__VA_ARGS__) + std::string("\n") ) 
//#else
//	#define LOG_ERROR(...) g_theConsole->Error( Stringf(__VA_ARGS__) );
//#endif
 
UDPSocket::UDPSocket()
	:m_timeval{0l, 0l}
{
	FD_ZERO( &m_fdSet );
}

UDPSocket::UDPSocket( const std::string& host, int port )
	:m_socket( INVALID_SOCKET )
	,m_timeval{0l, 0l}
{
	FD_ZERO( &m_fdSet );

	m_toAddress.sin_family = AF_INET;
	m_toAddress.sin_port = htons( (u_short)port );
	m_toAddress.sin_addr.s_addr = inet_addr( host.c_str() );

	m_socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( m_socket == INVALID_SOCKET )
	{
		g_theConsole->Error( "UDP Socket init failed, error = %i", WSAGetLastError() );
	}
}

UDPSocket::~UDPSocket()
{
	Close();
	FD_ZERO( &m_fdSet );

	m_isQuitting = true;

	if( m_readerThread )
	{
		m_readerThread->join();
		delete m_readerThread;
		m_readerThread = nullptr;
	}

	if( m_writerThread )
	{
		m_writerThread->join();
		delete m_writerThread;
		m_writerThread = nullptr;
	}
}

void UDPSocket::Bind( int port )
{
	m_bindAddress.sin_family = AF_INET;
	m_bindAddress.sin_port = htons( (u_short)port );
	m_bindAddress.sin_addr.s_addr = htonl( INADDR_ANY );
	 
	int result = ::bind( m_socket, (SOCKADDR*)&m_bindAddress, sizeof( m_bindAddress ) );
	if( result == SOCKET_ERROR )
	{
 		g_theConsole->Error( "Socket bind failed, error = %i", WSAGetLastError() );
	}
}

int UDPSocket::Send( int length )
{
	int result = ::sendto( m_socket, &m_sendBuffer[0], static_cast<int>(length), 
		0, reinterpret_cast<SOCKADDR*>(&m_toAddress), sizeof(m_toAddress) );
	if( result == SOCKET_ERROR )
	{
 		g_theConsole->Error( "Socket send failed, error = %i", WSAGetLastError() );
	}
	return result;
}

int UDPSocket::Receive()
{
	sockaddr_in fromAddr;
	int fromLength = sizeof( fromAddr );
	int result = ::recvfrom( m_socket, &m_receiveBuffer[0],
		static_cast<int>(m_receiveBuffer.size()), 0, reinterpret_cast<SOCKADDR*>(&fromAddr), &fromLength );
	if( result == SOCKET_ERROR || result < 0 )
	{
 		g_theConsole->Error( "Socket receive failed, error = %i", WSAGetLastError() );
	}

	char fromIp[256];
	ZeroMemory( fromIp, 256 );
	inet_ntop( AF_INET, &fromAddr.sin_addr, fromIp, 256 );
	g_theConsole->Printf( "Receive data from: %s", fromIp );

	//if( result != SOCKET_ERROR )
	//{
	//	MessageHeader* pHeader = reinterpret_cast< MessageHeader* >( &m_receiveBuffer[0] );
	//	m_receiveBuffer[ result ] = NULL;
	//}

	return result;
}

void UDPSocket::Close()
{
	if( m_socket != INVALID_SOCKET )
	{
		int result = closesocket( m_socket );
		if( result == SOCKET_ERROR )
		{
 			g_theConsole->Error( "Failed to close socket, error = %i", WSAGetLastError() );
		}
		m_socket = INVALID_SOCKET;
	}
}

bool UDPSocket::IsDataAvailable()
{
	FD_ZERO( &m_fdSet );
	FD_SET( m_socket, &m_fdSet );
	int iResult = select( 0, &m_fdSet, NULL, NULL, &m_timeval );
	if( iResult == SOCKET_ERROR )
	{
		closesocket( m_socket );
	}
	return FD_ISSET( m_socket, &m_fdSet );
}

void UDPSocket::SendMsg( eMessageType id, char const* pData, int length )
{
	switch( id ) {
		case eMessageType::TEXT_MESSAGE: 
		{
			MessageHeader* header = reinterpret_cast<MessageHeader*>( &m_sendBuffer[0] );
			header->m_id = (uint16_t)id;
			header->m_size = (uint16_t)length;

			std::string str( pData );
			for( int i = 0; i < str.size(); ++i )
			{
				m_sendBuffer[i + 4] = str[i];
			}

			Send( length + 4 );
			break;
		}

	}
}

void UDPSocket::PushMsgIntoQueue( const std::string& message )
{
	m_writerQueue.Push( message );
}

std::string UDPSocket::GetMsg()
{
	return m_readerQueue.Pop();
}

void UDPSocket::SetupThreads()
{
	m_writerThread = new std::thread( &UDPSocket::WriterThreadMain, this );
	m_readerThread = new std::thread( &UDPSocket::ReaderThreadMain, this );
}

void UDPSocket::WriterThreadMain()
{
	while( !m_isQuitting )
	{
		std::string messageToSend = m_writerQueue.Pop();
		if( !messageToSend.empty() )
		{
			int stringLength = (int)messageToSend.length();
			std::memcpy( &m_sendBuffer[0], &messageToSend[0], stringLength );
			SendMsg( eMessageType::TEXT_MESSAGE, messageToSend.c_str(), (int)messageToSend.length() );
			//Send( stringLength );
		}
	}

	g_theConsole->Printf( "Exiting writer thread..." );
}

void UDPSocket::ReaderThreadMain()
{
	while( !m_isQuitting )
	{
		if( IsDataAvailable() )
		{
			int length = Receive();
			if( length > 0 )
			{
				MessageHeader* pHeader = reinterpret_cast<MessageHeader*>(&m_receiveBuffer[0]);
				m_receiveBuffer[length] = NULL;
				TCPMessageData messageData( *pHeader, &m_receiveBuffer[4] );

				switch( pHeader->m_id ) {
					case 2: // Text Message
					{
						std::string msg = &m_receiveBuffer[4];
						m_readerQueue.Push( msg );
						break;
					}
				}

				//std::string msg = &m_receiveBuffer[0];
				
			}
		}
	}
	// Hack to avoid thread blocking.
	// ToDo: Remove this
	m_writerQueue.Push( "" );

	g_theConsole->Printf( "Exiting reader thread..." );
}
