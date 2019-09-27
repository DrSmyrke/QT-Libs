#ifndef NTP_H
#define NTP_H

#include <QObject>
#include <QUdpSocket>
#include <QDateTime>
#include "global.h"
#include "myfunctions.h"


struct NtpTimestamp{
	unsigned int seconds;	// Текущее время в UNIX time
	unsigned int fraction;	// Доли секунды
};

struct NtpPacket{
	unsigned char mode: 3;
	unsigned char versionNumber: 3;
	unsigned char leapIndicator: 2;
	unsigned char stratum;
	unsigned char poll;
	unsigned char precision;
	unsigned int rootDelay;
	unsigned int rootDispersion;
	unsigned char referenceID[4];
	NtpTimestamp referenceTimestamp;
	NtpTimestamp originateTimestamp;
	NtpTimestamp receiveTimestamp;
	NtpTimestamp transmitTimestamp;
};



class NtpClient : public QObject
{
	Q_OBJECT
public:
	explicit NtpClient(QObject *parent = nullptr);
	bool init();
	void sync();
	bool isError(){ return ( m_errorCount > 0 ) ? true : false; }
	uint32_t getTime(){ return m_dateTime.toTime_t(); }
signals:
	void signal_error();
	void signal_getTime();
private slots:
	void slot_readyRead();
private:
	QUdpSocket* m_pSocket;
	bool m_init;
	QDateTime m_dateTime;
	uint8_t m_errorCount;
};

#endif // NTP_H
