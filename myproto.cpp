#include "myproto.h"
#include "../myfunctions.h"

namespace myproto {
	DefaultConfig conf;
	AppData appData;

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

		if( pkt.preamble != preamble ){
			data.remove( 0, 1 );
			pkt.retry = true;
			return pkt;
		}

		pkt.chanelNum = data[4];

		pkt.destination = data[5]<<12;
		pkt.destination += data[6]<<8;
		pkt.destination += data[7]<<4;
		pkt.destination += data[8];

		pkt.dataLength = data[9]<<12;
		pkt.dataLength += data[10]<<8;
		pkt.dataLength += data[11]<<4;
		pkt.dataLength += data[12];

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

		ba.append( mf::toBigEndianInt( preamble ) );
		ba.append( pkt.chanelNum );
		ba.append( mf::toBigEndianInt( pkt.destination ) );
		ba.append( mf::toBigEndianInt( pkt.rawData.size() ) );
		ba.append( pkt.rawData );
		ba.append( mf::toBigEndianShort( myproto::getCRC( ba ) ) );

		return ba;
	}

	void parsParams(Pkt &pkt)
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

	void setParam(QByteArray &data, const uint16_t param, const QByteArray &value)
	{
		data.append( mf::toBigEndianShort( param ) );
		data.append( mf::toBigEndianShort( value.size() ) );
		data.append( value );
	}

	uint16_t getCRC(const QByteArray &data)
	{
		uint16_t crc = 0;
		for( auto sym:data ) crc += sym;
		return crc;
	}

	Pkt getPkt( const uint8_t chanelNum, const uint8_t type )
	{
		Pkt pkt;

		pkt.chanelNum = chanelNum;
		pkt.destination = 0;

		switch (type) {
			case pkt_type_helo:		setParam( pkt.rawData, param_type_helo, "Hello" );		break;
			case pkt_type_bye:		setParam( pkt.rawData, param_type_bye );				break;
		}

		return pkt;
	}

}
