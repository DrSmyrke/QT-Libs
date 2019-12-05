#include "myproto.h"
#include "myfunctions.h"

namespace myproto {
	const uint32_t preamble = 0xDEADBEEF;

	myproto::Pkt parsPkt(QByteArray &data)
	{
		myproto::Pkt pkt;

		if( data.size() < pkt.headerSize ){
			pkt.next = true;
			return pkt;
		}

		pkt.head.preamble = static_cast<uint8_t>( data[0] )<<24;
		pkt.head.preamble += static_cast<uint8_t>( data[1] )<<16;
		pkt.head.preamble += static_cast<uint8_t>( data[2] )<<8;
		pkt.head.preamble += static_cast<uint8_t>( data[3] );

		if( pkt.head.preamble != myproto::preamble ){
			data.remove( 0, 1 );
			pkt.retry = true;
			return pkt;
		}

		pkt.head.channel = static_cast<uint8_t>( data[4] );
		pkt.head.type = static_cast<uint8_t>( data[5] );

		pkt.head.source = static_cast<uint8_t>( data[6] )<<24;
		pkt.head.source += static_cast<uint8_t>( data[7] )<<16;
		pkt.head.source += static_cast<uint8_t>( data[8] )<<8;
		pkt.head.source += static_cast<uint8_t>( data[9] );
		
		pkt.head.destination = static_cast<uint8_t>( data[10] )<<24;
		pkt.head.destination += static_cast<uint8_t>( data[11] )<<16;
		pkt.head.destination += static_cast<uint8_t>( data[12] )<<8;
		pkt.head.destination += static_cast<uint8_t>( data[13] );

		pkt.head.dataLength = static_cast<uint8_t>( data[14] )<<24;
		pkt.head.dataLength += static_cast<uint8_t>( data[15] )<<16;
		pkt.head.dataLength += static_cast<uint8_t>( data[16] )<<8;
		pkt.head.dataLength += static_cast<uint8_t>( data[17] );

		uint32_t totalSize = pkt.head.dataLength + pkt.headerSize + sizeof(pkt.crc);

		if( data.size() < totalSize ){
			pkt.next = true;
			return pkt;
		}

		pkt.rawData.append( data.mid( pkt.headerSize, pkt.head.dataLength ) );

		pkt.crc += static_cast<uint8_t>( data[pkt.head.dataLength + pkt.headerSize] )<<8;
		pkt.crc += static_cast<uint8_t>( data[pkt.head.dataLength + pkt.headerSize + 1] );

		if( pkt.crc != myproto::getCRC( data.left( pkt.head.dataLength + pkt.headerSize ) ) ){
			data.remove( 0, totalSize );
			pkt.error = true;
			return pkt;
		}

		data.remove( 0, totalSize );

		return pkt;
	}

	QByteArray buidPkt(const Pkt &pkt, const QByteArray &key)
	{
		QByteArray ba;

		ba.append( mf::toBigEndianInt( myproto::preamble ) );
		ba.append( pkt.head.channel );
		ba.append( pkt.head.type );
		ba.append( mf::toBigEndianInt( pkt.head.source ) );
		ba.append( mf::toBigEndianInt( pkt.head.destination ) );
		ba.append( mf::toBigEndianInt( pkt.rawData.size() ) );
		if( key.size() > 0 ) mf::XOR( pkt.rawData, key );
		ba.append( pkt.rawData );
		ba.append( mf::toBigEndianShort( myproto::getCRC( ba ) ) );

		return ba;
	}

	void parsData(Pkt &pkt, const QByteArray &key)
	{
		if( pkt.rawData.size() == 0 ) return;
		if( key.size() > 0 ) mf::XOR( pkt.rawData, key );

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
					pkt.pktData.push_back( pktData );
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

	uint16_t getCRC(const QByteArray &data)
	{
		uint16_t crc = 0;
		for( auto sym:data ) crc += sym;
		return crc;
	}

	QByteArray findData(const Pkt &pkt, const uint16_t dataType)
	{
		QByteArray ba;
		for( auto elem:pkt.pktData ){
			if( elem.type == dataType ){
				ba.append( elem.data );
				break;
			}
		}
		return ba;
	}

}
