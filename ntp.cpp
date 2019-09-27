#include "ntp.h"
#include <sys/time.h>
#include <QtEndian>

NtpClient::NtpClient(QObject *parent)
	: QObject(parent)
	, m_init(false)
	, m_errorCount(0)
{
	m_pSocket = new QUdpSocket(this);

	connect( m_pSocket, &QUdpSocket::readyRead, this, &NtpClient::slot_readyRead );

	app::setLog( 1, "NTP CLIENT CREATED" );
}

bool NtpClient::init()
{
	m_init = m_pSocket->bind( QHostAddress::AnyIPv4 );

	if( m_init ){
		app::setLog( 3, "NTP CLIENT INIT OK" );
	}else{
		app::setLog( 2, "NTP CLIENT INIT ERROR" );
	}

	return m_init;
}

void NtpClient::sync()
{
	if( !m_init ){
		app::setLog( 2, "NTP CLIENT is NOT inited !!!" );
		return;
	}

	app::setLog( 3, QString("NTP SYNC ...") );

	NtpPacket packet;
	memset(&packet, 0, sizeof(packet));
	packet.mode = 3;
	packet.versionNumber = 3;
	//timeval t;
	//gettimeofday( &t, nullptr );
	//packet.transmitTimestamp.seconds = t.tv_sec;
	//packet.transmitTimestamp.fraction = t.tv_usec;

	uint16_t len = sizeof(packet);
	char* pkt = reinterpret_cast<char *> (&packet);
	QByteArray buff;
	for( uint16_t i = 0; i < len; i++ ) buff.append( pkt[i] );

	for( auto elem:app::conf.ntp.servers ){
		int res = m_pSocket->writeDatagram( buff.data(), buff.size(), elem.ip, elem.port );
		m_pSocket->waitForBytesWritten( 100 );
		app::setLog( 5, QString("NTP CLIENT SEND to [%1:%2] [%3] [%4]").arg( elem.ip.toString() ).arg( QString::number( elem.port ) ).arg( QString::number( res ) ).arg( QString( buff.toHex() ) ));
	}

	if( m_errorCount < app::conf.ntp.errorMaxCount ){
		m_errorCount++;
	}else{
		emit signal_error();
	}
}

void NtpClient::slot_readyRead()
{
	if( !m_init ) return;
	m_errorCount = 0;

	while( m_pSocket->hasPendingDatagrams() ){
		QByteArray buff;
		buff.resize(m_pSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		m_pSocket->readDatagram( buff.data(), buff.size(), &sender, &senderPort );


		NtpPacket* pkt = reinterpret_cast<struct NtpPacket*>(buff.data());
		unsigned long long NTP_TIMESTAMP_DELTA = 2208988800ull;

		//time_t txTm = ( time_t ) ( qToBigEndian(pkt->transmitTimestamp.seconds) - NTP_TIMESTAMP_DELTA );
		//printf( "Time: %s\n", ctime( ( const time_t* ) &txTm ) );

		m_dateTime.setTime_t( qToBigEndian(pkt->transmitTimestamp.seconds) - NTP_TIMESTAMP_DELTA );

		app::setLog( 5, QString("NTP CLIENT RCV from [%1:%2] [%3]").arg( sender.toString() ).arg( QString::number( senderPort ) ).arg( QString( buff.toHex() ) ));
		app::setLog( 4, QString("NTP CLIENT RCV dateTime [%1]").arg( m_dateTime.toString("dd.MM.yyyy [hh:mm:ss]") ));

		mf::startDetached( "date", QStringList()<<m_dateTime.toString("MMddhhmmyyyy.ss") );

		emit signal_getTime();
	}
}
