#include "myproto.h"
#include "myfunctions.h"

namespace myproto {
	const uint32_t preamble = 0xDEADBEEF;

	Pkt parsPkt(QByteArray &data)
	{
		Pkt pkt;
		pkt.headerSize = sizeof( myproto::Pkt::Head );

		if( data.size() < pkt.headerSize ){
			pkt.next = true;
			return pkt;
		}

		pkt.head.preamble = data[0]<<12;
		pkt.head.preamble += data[1]<<8;
		pkt.head.preamble += data[2]<<4;
		pkt.head.preamble += data[3];

		if( pkt.head.preamble != myproto::preamble ){
			data.remove( 0, 1 );
			pkt.retry = true;
			return pkt;
		}

		pkt.head.channel = data[4];
		pkt.head.type = data[5];

		pkt.head.source = data[6]<<12;
		pkt.head.source += data[7]<<8;
		pkt.head.source += data[8]<<4;
		pkt.head.source += data[9];
		
		pkt.head.destination = data[10]<<12;
		pkt.head.destination += data[11]<<8;
		pkt.head.destination += data[12]<<4;
		pkt.head.destination += data[13];

		pkt.head.dataLength = data[14]<<12;
		pkt.head.dataLength += data[15]<<8;
		pkt.head.dataLength += data[16]<<4;
		pkt.head.dataLength += data[17];

		uint32_t totalSize = pkt.head.dataLength + pkt.headerSize + sizeof(pkt.crc);

		if( data.size() < totalSize ){
			pkt.next = true;
			return pkt;
		}

		pkt.rawData.append( data.mid( pkt.headerSize, pkt.head.dataLength ) );

		pkt.crc += data[pkt.head.dataLength + pkt.headerSize]<<4;
		pkt.crc += data[pkt.head.dataLength + pkt.headerSize + 1];

		if( pkt.crc != myproto::getCRC( data.left( pkt.head.dataLength + pkt.headerSize ) ) ){
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
		ba.append( pkt.head.channel );
		ba.append( mf::toBigEndianInt( pkt.head.destination ) );
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
}
