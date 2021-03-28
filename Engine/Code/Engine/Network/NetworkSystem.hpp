#pragma once
#pragma warning(disable: 6319)

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Network/UDPSocket.hpp"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdint>
#include <string>
#include <vector>

#pragma comment(lib,"ws2_32.lib")

class TCPServer;
class TCPClient;

enum class eRole
{
	INVALID = -1,

	SERVER,
	CLIENT,
	UDP,
};

class NetworkSystem
{
public:
	NetworkSystem();
	~NetworkSystem();

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	bool GetIsListening() { return m_isListening; }
	void SetIsListening( bool listen );

	void CreateTCPServer( int port = 48000 );
	void CreateTCPClient( int port = 48000 );
	void SetRole( eRole role );
	void SendMessageIfDataAvailable();

	void StopServer();
	void ClientDisconnect();


	//-------------------------------------------------------------------------------------------------------------
	// UDP
	//-------------------------------------------------------------------------------------------------------------
	void		CreateUDPSocket( int bindPort );
	UDPSocket*	GetUDPSocket() { return m_UDPSocket; }
	void		DisconnectUDP();


public:
	bool		m_isListening = false;
	eRole		m_role = eRole::INVALID;
	TCPServer*	m_tcpServer = nullptr;
	TCPClient*	m_tcpClient = nullptr;
	TCPSocket	m_listenSocket;
	SOCKET		m_clientSocket;

	UDPSocket*  m_UDPSocket = nullptr;

	std::vector<std::string> m_messagesWaitingToBeSent;
};