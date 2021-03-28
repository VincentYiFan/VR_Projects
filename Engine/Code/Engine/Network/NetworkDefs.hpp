#pragma once
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

enum class Mode
{
	Invalid = 0,
	Blocking = 1,
	Nonblocking = 2
};

enum class eMessageType : int
{
	SERVER_LISTENING = 1,
	TEXT_MESSAGE,
	CLIENT_DISCONNECTING
};

struct MessageHeader
{
	MessageHeader() {};
	MessageHeader( uint16_t id, uint16_t size )
		: m_id( id )
		, m_size( size ) {};

	uint16_t	m_id = 0;
	uint16_t	m_size = 0;
};

struct TCPMessageData
{
	TCPMessageData() {};
	TCPMessageData( MessageHeader messageHeader, std::string gameName )
		: m_header( messageHeader )
		, m_gameName( gameName ) {};

	MessageHeader	m_header;
	std::string		m_gameName;
};
