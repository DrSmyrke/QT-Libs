#ifndef MYPROTO_H
#define MYPROTO_H

#include <QByteArray>
#include <list>

/*
 * Packet Structure
 *
 * preamble					4 byte
 * channel					1 byte
 * source					4 byte
 * destination				4 byte		if destination = 0xFFFFFFFF then this message from all
 * dataLength				4 byte
 * format/encrypt data		(dataLength) bytes
 * crc						4 byte
 * -------------------------------------------------------
 * total header				17 bytes
 * total packet				21 + (dataLength) bytes
*/


namespace myproto {
	struct Channel{
		enum{
			comunication				= 1,
			auth,
			error,
		};
	};
	struct PktType{
		enum{
			hello,
			bye,
		};
	};
	struct DataType{
		enum{
			empty,
			text,
			version,
			buffSize,
			name,
		};
	};
	struct PktData {
		uint16_t type					= myproto::DataType::empty;
		uint16_t size					= 0;
		QByteArray data;
	};
	struct Pkt{
		uint32_t preamble				= 0;
		uint8_t channel					= 0;
		uint32_t source					= 0;
		uint32_t destination			= 0;
		uint32_t dataLength				= 0;
		QByteArray rawData;
		uint16_t crc					= 0;
		std::list<myproto::PktData> pktData;
		const uint8_t headerSize		= 18;
		bool next = false;
		bool error = false;
		bool retry = false;
	};

	Pkt parsPkt( QByteArray &data );
	QByteArray buidPkt( const Pkt &pkt );
	void parsData( Pkt &pkt );
	void addData( QByteArray &data, const uint16_t param, const QByteArray &value = QByteArray() );
	uint32_t getCRC( const QByteArray &data );
	Pkt getPkt(const uint8_t type );
}

#endif // MYPROTO_H
