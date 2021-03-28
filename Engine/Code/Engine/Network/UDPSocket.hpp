#pragma once
#include "Engine/Network/SynchronizedBlockingQueue.h"
#include "Engine/Network/SynchronizedNonBlockingQueue.h"
#include "Engine/Network/NetworkDefs.hpp"
#include <string>
#include <array>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class UDPSocket
{
public:
	static const int BufferSize = 512;
	using Buffer = std::array< char, BufferSize >;

	UDPSocket();
	UDPSocket( const std::string& host, int port );
	~UDPSocket();

	bool	IsValid() { return m_socket != INVALID_SOCKET; }

	void	Bind( int port );
	int		Send( int length );
	int		Receive();
	void	Close();

	bool	IsDataAvailable();

	void			SendMsg( eMessageType id, char const* pData, int legnth );
	void			PushMsgIntoQueue( const std::string& message );
	std::string		GetMsg();

	void	SetupThreads();

	void	WriterThreadMain();
	void	ReaderThreadMain();

	Buffer& GetSendBuffer() { return m_sendBuffer; }
	Buffer& GetReceiveBuffer() { return m_receiveBuffer; }

private:
	bool			m_isQuitting = false;

	Buffer			m_sendBuffer;
	Buffer			m_receiveBuffer;
	sockaddr_in		m_toAddress;
	sockaddr_in		m_bindAddress;
	SOCKET			m_socket;

	// reader - nonblocking
	// writer - blocking
	SynchronizedNonBlockingQueue<std::string> m_readerQueue;
	SynchronizedBlockingQueue<std::string> m_writerQueue;

	std::thread* m_writerThread = nullptr;
	std::thread* m_readerThread = nullptr;

	FD_SET			m_fdSet;
	timeval			m_timeval;
};