#pragma once
#include "Engine/Network/NetworkDefs.hpp"
#include <string>

#pragma warning(disable: 6319)

// Use std::max template not macro since it conflicts
// with max function in numeric_limits.
#ifdef max
#undef max
#endif


class TCPSocket
{
public:
	TCPSocket();
	TCPSocket( SOCKET socket, Mode mode = Mode::Nonblocking, std::size_t bufferSize = 256 );
	TCPSocket(const TCPSocket& src);
	~TCPSocket();

	TCPSocket& operator=(const TCPSocket& src);

	std::size_t			GetBufferSize() { return m_bufferSize; }
	bool				IsValid() { return m_socket != INVALID_SOCKET; }
	std::string			GetAddress();

	void				Send( char const* dataPtr, std::size_t length );
	void				SendMsg( eMessageType id, char const* dataPtr, std::size_t length );

	TCPMessageData		Receive();

	bool				IsDataAvailable();

	void				ShutDown();
	void				Close();

	SOCKET				GetSocket() { return m_socket; }
	

private:
	Mode			m_mode;
	SOCKET			m_socket;

	FD_SET			m_fdSet;
	timeval			m_timeval;

	std::size_t		m_bufferSize;
	std::size_t		m_receiveSize;
	char*			m_bufferPtr;
};

