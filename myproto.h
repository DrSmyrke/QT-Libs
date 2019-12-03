#ifndef MYPROTO_H
#define MYPROTO_H

#include <QByteArray>
#include <list>

enum{
	socket_type_client,
	socket_type_server,
};

enum{
	app_type_noname,
	app_type_server,
	app_type_appcenter,
	app_type_mystats,
};

enum{
	param_type_empty,
	param_type_helo,
	param_type_name,
	param_type_where_are_you,
	param_type_i_am_here,
	param_type_bye,
};

enum{
	pkt_type_helo,
	pkt_type_bye,
};

enum{
	pkt_channel_comunication		= 1,
	pkt_channel_auth,
};

struct PktData
{
	uint16_t type = param_type_empty;
	uint16_t size = 0;
	QByteArray data;
};

/*
 * Packet Structure
 *
 * preamble					4 byte
 * channelNum				1 byte
 * destination				4 byte		if destination = 0xFFFFFFFF then this message from all
 * dataLength				4 byte
 * format/encrypt data		(dataLength) bytes
 * crc						2 byte
 * -------------------------------------------------------
 * total header				13 bytes
 * total packet				15 + (dataLength) bytes
*/

struct Pkt{
	uint32_t preamble				= 0;
	uint8_t chanelNum				= 0;
	uint32_t destination			= 0;
	uint32_t dataLength				= 0;
	QByteArray rawData;
	uint16_t crc					= 0;
	std::list<PktData> pktData;
	const uint8_t headerSize		= 13;
	bool next = false;
	bool error = false;
	bool retry = false;
};

struct DefaultConfig
{
	uint16_t port		= 7300;
};

struct AppData
{
	uint8_t type = app_type_noname;
	std::list<PktData> params;
	uint32_t ip;
	QByteArray id;
	bool confirm = false;
	bool connected = false;
};

const uint32_t preamble = 0xDEADBEEF;

namespace myproto {
	extern DefaultConfig conf;
	extern AppData appData;

	Pkt parsPkt( QByteArray &data );
	QByteArray buidPkt( const Pkt &pkt );
	void parsParams( Pkt &pkt );
	void setParam( QByteArray &data, const uint16_t param, const QByteArray &value = QByteArray() );
	uint16_t getCRC( const QByteArray &data );
	Pkt getPkt( const uint8_t chanelNum, const uint8_t type );
}

#endif // MYPROTO_H
