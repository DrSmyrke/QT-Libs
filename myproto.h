#ifndef MYPROTO_H
#define MYPROTO_H

#include <QByteArray>
#include <list>

/*
 * Packet Structure
 *
 * preamble					4 byte
 * channel					1 byte
 * type						1 byte
 * source					4 byte
 * destination				4 byte		if destination = 0xFFFFFFFF then this message from all
 * dataLength				4 byte
 * format/encrypt data		(dataLength) bytes
 * crc						4 byte
 * -------------------------------------------------------
 * total header				18 bytes
 * total packet				22 + (dataLength) bytes
*/


namespace myproto {
	struct Channel{
		enum{
			comunication				= 1,
			auth,
		};
	};
	struct PktType{
		enum{
			hello,
			hello2,
			request,
			response,
			error,
			bye,
		};
	};
	struct DataType{
		enum{
			empty,
			text,
			url,
			id,
			login,
			version,
			buffSize,
			boolean,
			name,
		};
	};
	struct PktData {
		uint16_t type						= myproto::DataType::empty;
		uint16_t size						= 0;
		QByteArray data;
	};
	struct Pkt{
		struct Head{
			uint32_t preamble				= 0;
			uint8_t channel					= 0;
			uint8_t type					= 0;
			uint32_t source					= 0;
			uint32_t destination			= 0;
			uint32_t dataLength				= 0;
		} head;
		QByteArray rawData;
		uint16_t crc						= 0;
		uint8_t headerSize					= 18;
		std::list<myproto::PktData> pktData;
		bool next							= false;
		bool error							= false;
		bool retry							= false;
	};

	myproto::Pkt parsPkt( QByteArray &data );
	QByteArray buidPkt(const myproto::Pkt &pkt);
	QByteArray buidPkt(myproto::Pkt &pkt, const QByteArray &key);
	void parsData(myproto::Pkt &pkt);
	void parsData(myproto::Pkt &pkt, const QByteArray &key);
	void addData(QByteArray &data, const uint16_t param, const QByteArray &value = QByteArray());
	uint16_t getCRC(const QByteArray &data);
	QByteArray findData(const myproto::Pkt &pkt, const uint16_t dataType);
}

#endif // MYPROTO_H
