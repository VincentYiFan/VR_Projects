#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Network/TCPServer.hpp"
#include "Engine/Network/TCPClient.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <array>

//-------------------------------------------------------------------------------------------------------------
COMMAND( StartTCPServer, "port" )
{
	int port = args.GetValue( "port", 48000 );
	g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Start TCP server listening for client connections, port=%i", port ) );
	g_theInput->SetCursorMode( MOUSE_MODE_ABSOLUTE );

	g_theNetwork->SetRole( eRole::SERVER );
	g_theNetwork->CreateTCPServer( port );
	g_theNetwork->SetIsListening( true );
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( StartTCPClient, "ip,port" )
{
	int port = args.GetValue( "port", 48000 );
	g_theInput->SetCursorMode( MOUSE_MODE_ABSOLUTE );

	g_theNetwork->SetRole( eRole::CLIENT );
	g_theNetwork->CreateTCPClient( port );
	g_theNetwork->SetIsListening( true );

	g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Start TCP client connecting to server, port=%i", port ) );
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( SendMessage, "msg" )
{
	std::string message = args.GetValue( "msg", "" );
	g_theNetwork->m_messagesWaitingToBeSent.push_back( message );
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( StopServer, "" )
{
	UNUSED( args );
	g_theConsole->PrintString( Rgba8::GREEN, "Stop Server..." );
	//g_theNetwork->StopServer();
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( Disconnect, "" )
{
	UNUSED( args );
	g_theConsole->PrintString( Rgba8::GREEN, "Disconnected!" );
	std::string message ("    Cient Disconnected");
	g_theNetwork->m_listenSocket.Send( message.c_str(), message.length() );
	//g_theNetwork->ClientDisconnect();
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( OpenUDPPort, "bindPort,sendToPort" )
{
	std::string bindPort = args.GetValue( "bindPort", "" );
	std::string sendToPort = args.GetValue( "sendToPort", "" );
	g_theConsole->Printf( "Starting UDPSocket with bindPort=%s, sendToPort=%s", bindPort.c_str(), sendToPort.c_str() );

	int bindPortNumber = args.GetValue( "bindPort", 48000 );
	int sendToPortNumber = args.GetValue( "sendToPort", 48001 );
	g_theNetwork->CreateUDPSocket( bindPortNumber );
	g_theNetwork->GetUDPSocket()->Bind( sendToPortNumber );
	g_theNetwork->GetUDPSocket()->SetupThreads();

	g_theNetwork->SetRole( eRole::UDP );
	g_theNetwork->SetIsListening( true );
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( SendUDPMessage, "msg" )
{
	std::string message = args.GetValue( "msg", "" );

	g_theNetwork->GetUDPSocket()->PushMsgIntoQueue( message );
}

//-------------------------------------------------------------------------------------------------------------
COMMAND( CloseUDPPort, "bindPort" )
{
	UNUSED( args );
	g_theNetwork->DisconnectUDP();
}

//-------------------------------------------------------------------------------------------------------------
NetworkSystem::NetworkSystem()
	:m_isListening( false )
	,m_clientSocket( INVALID_SOCKET )
{
	/*FD_ZERO( &m_listenSet );*/
}

//-------------------------------------------------------------------------------------------------------------
NetworkSystem::~NetworkSystem()
{
	m_isListening = false;
	m_clientSocket = INVALID_SOCKET;

	if( m_UDPSocket )
	{
		delete m_UDPSocket;
		m_UDPSocket = nullptr;
	}
}

//-------------------------------------------------------------------------------------------------------------
void NetworkSystem::StartUp()
{
	// Initialize winsock
	WSADATA wsaData;
	WORD wVersion MAKEWORD( 2, 2 );
	int iResult = WSAStartup( wVersion, &wsaData );
	if( iResult != 0 )
	{
		g_theConsole->Error( "ERROR: Call to WSAStartup failed: %i", WSAGetLastError() );
	}
}

//-------------------------------------------------------------------------------------------------------------
void NetworkSystem::BeginFrame()
{
	if( m_isListening )
	{
		switch( m_role )
		{
		case eRole::SERVER:
		{
			if( m_tcpServer != nullptr )
			{ 
				if( !m_listenSocket.IsValid() )
				{
					m_listenSocket = m_tcpServer->Accept();
					if( m_listenSocket.IsValid() )
					{
						g_theConsole->PrintString( Rgba8::GREEN, Stringf( "Client connected from %s", m_listenSocket.GetAddress().c_str() ) );
					}
				}
				else
				{
					TCPMessageData data = m_listenSocket.Receive();
					if( data.m_header.m_id != 0 )
					{
						g_theConsole->PrintString( Rgba8::WHITE, data.m_gameName ); 
					}	

					SendMessageIfDataAvailable();
				}
			}
		}
		break;

		case eRole::CLIENT:
		{
			if( !m_listenSocket.IsValid() )
			{
				m_listenSocket = m_tcpClient->Connect( std::string( "VincentTsou" ), (uint16_t)m_tcpClient->GetPort() );
			}
			else
			{
				if( m_listenSocket.IsDataAvailable() )
				{
					TCPMessageData data = m_listenSocket.Receive();
					if( data.m_header.m_id != 0 )
					{
						g_theConsole->PrintString( Rgba8::WHITE, data.m_gameName );
					}
				}

				SendMessageIfDataAvailable();
			}

		}
		break;

		case eRole::UDP:
		{
			if( m_UDPSocket && m_UDPSocket->IsValid() )
			{
				std::string messageStr = m_UDPSocket->GetMsg();
				if( !messageStr.empty() )
				{
					g_theConsole->PrintString( Rgba8::CYAN, messageStr );
				}
			}
		}
		break;
		}

	}
}

//-------------------------------------------------------------------------------------------------------------
void NetworkSystem::EndFrame()
{
}

//-------------------------------------------------------------------------------------------------------------
void NetworkSystem::ShutDown()
{
	int iResult = WSACleanup();
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->Error( "Call to WSACleanup failed: %i", WSAGetLastError() );
	}
}

//-------------------------------------------------------------------------------------------------------------
void NetworkSystem::SetIsListening( bool listen )
{
	m_isListening = listen;
}

//-------------------------------------------------------------------------------------------------------------
void NetworkSystem::CreateTCPServer( int port )
{
	if( nullptr == m_tcpServer )
	{
		m_tcpServer = new TCPServer( port );
		m_tcpServer->m_timeval.tv_sec = 0;
		m_tcpServer->m_timeval.tv_usec = 1;
		if( m_tcpServer != nullptr )
		{
			m_tcpServer->Bind();
			m_tcpServer->Listen();
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
void NetworkSystem::CreateTCPClient( int port )
{
	if( m_tcpClient == nullptr )
	{
		m_tcpClient = new TCPClient( port );
	}
}

void NetworkSystem::SetRole( eRole role )
{
	m_role = role;
}

void NetworkSystem::SendMessageIfDataAvailable()
{
	for( int i = 0; i < (int)m_messagesWaitingToBeSent.size(); ++i )
	{
		if( m_messagesWaitingToBeSent[i] != "" )
		{
			std::string msgStr = m_messagesWaitingToBeSent[i];

			m_listenSocket.SendMsg( eMessageType::TEXT_MESSAGE, &msgStr[0], msgStr.length() );

			m_messagesWaitingToBeSent[i] = "";
		}
	}
}

void NetworkSystem::StopServer()
{
	SetIsListening(false);
	if( m_tcpServer != nullptr )
	{
		m_tcpServer->StopListen();
		if( m_listenSocket.IsValid() )
		{
			m_listenSocket.ShutDown();
			m_listenSocket.Close();
		}
	}
}

void NetworkSystem::ClientDisconnect()
{
	SetIsListening(false);
	if( m_listenSocket.IsValid() )
	{
		m_listenSocket.ShutDown();
		m_listenSocket.Close();
	}	
}

void NetworkSystem::CreateUDPSocket( int bindPort )
{
	if( m_UDPSocket )
	{
		return;
	}

	m_UDPSocket = new UDPSocket( "127.0.0.1", bindPort );
	if( !m_UDPSocket )
	{
		g_theConsole->Error( "Failed to create UDPSocket" );
	}
}

void NetworkSystem::DisconnectUDP()
{
	if( m_UDPSocket )
	{
		m_UDPSocket->Close();

		delete m_UDPSocket;
		m_UDPSocket = nullptr;
	}
}
