#pragma once
#include "Engine/Network/TCPSocket.hpp"

class TCPClient
{
	friend class NetworkSystem;
protected:
	TCPClient( int port = 48000 )
		: m_port( port )
		, m_mode( Mode::Nonblocking ) {}

public:
	~TCPClient() {}

	TCPSocket Connect(
		std::string const& host,
		std::uint16_t port,
		Mode mode = Mode::Nonblocking
		);

	int GetPort() const { return m_port; }

private:
	int		m_port = 0; 
	Mode	m_mode;
};