#include "myproto.h"
#include "myfunctions.h"

namespace myproto {
	const uint32_t preamble = 0xDEADBEEF;

	Pkt parsPkt(QByteArray &data)
	{
		Pkt pkt;

		if( data.size() < pkt.headerSize ){
			pkt.next = true;
			return pkt;
		}

		pkt.preamble = data[0]<<12;
		pkt.preamble += data[1]<<8;
		pkt.preamble += data[2]<<4;
		pkt.preamble += data[3];

		if( pkt.preamble != myproto::preamble ){
			data.remove( 0, 1 );
			pkt.retry = true;
			return pkt;
		}

		pkt.channel = data[4];

		pkt.source = data[5]<<12;
		pkt.source += data[6]<<8;
		pkt.source += data[7]<<4;
		pkt.source += data[8];
		
		pkt.destination = data[9]<<12;
		pkt.destination += data[10]<<8;
		pkt.destination += data[11]<<4;
		pkt.destination += data[12];

		pkt.dataLength = data[13]<<12;
		pkt.dataLength += data[14]<<8;
		pkt.dataLength += data[15]<<4;
		pkt.dataLength += data[16];

		uint32_t totalSize = pkt.dataLength + pkt.headerSize + sizeof(pkt.crc);

		if( data.size() < totalSize ){
			pkt.next = true;
			return pkt;
		}

		pkt.rawData.append( data.mid( pkt.headerSize, pkt.dataLength ) );

		pkt.crc += data[pkt.dataLength + pkt.headerSize]<<4;
		pkt.crc += data[pkt.dataLength + pkt.headerSize + 1];

		if( pkt.crc != myproto::getCRC( data.left( pkt.dataLength + pkt.headerSize ) ) ){
			data.remove( 0, totalSize );
			pkt.error = true;
			return pkt;
		}

		data.remove( 0, totalSize );

		return pkt;
	}

	QByteArray buidPkt(const Pkt &pkt)
	{
		QByteArray ba;

		ba.append( mf::toBigEndianInt( myproto::preamble ) );
		ba.append( pkt.channel );
		ba.append( mf::toBigEndianInt( pkt.destination ) );
		ba.append( mf::toBigEndianInt( pkt.rawData.size() ) );
		ba.append( pkt.rawData );
		ba.append( mf::toBigEndianShort( myproto::getCRC( ba ) ) );

		return ba;
	}

	void parsData(Pkt &pkt)
	{
		while( pkt.rawData.size() >= 4 ){
			PktData pktData;
			pktData.type = pkt.rawData[0]<<4;
			pktData.type += pkt.rawData[1];
			pktData.size = pkt.rawData[2]<<4;
			pktData.size += pkt.rawData[3];

			pkt.rawData.remove( 0, 4 );

			if( pktData.size > 0 ){
				if( pkt.rawData.size() >= pktData.size ){
					pktData.data.append( pkt.rawData.left( pktData.size ) );
					pkt.rawData.remove( 0, pktData.size );
				}
			}
		}
	}

	void addData(QByteArray &data, const uint16_t param, const QByteArray &value)
	{
		data.append( mf::toBigEndianShort( param ) );
		data.append( mf::toBigEndianShort( value.size() ) );
		data.append( value );
	}

	uint32_t getCRC(const QByteArray &data)
	{
		uint16_t crc = 0;
		for( auto sym:data ) crc += sym;
		return crc;
	}

	Pkt getPkt(const uint8_t type)
	{
		Pkt pkt;

		switch (type) {
			case myproto::PktType::hello:
				pkt.channel = myproto::Channel::comunication;
				addData( pkt.rawData, myproto::DataType::text, "Hello" );
			break;
		}

		return pkt;
	}
}
