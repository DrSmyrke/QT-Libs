#ifndef WGET_H
#define WGET_H

#include <QObject>
#include <QTcpSocket>

struct RequestLine{
	QString proto = "HTTP/1.1";
	QString url;
	QString type;
};
struct ResponseLine{
	QString proto = "";
	uint16_t code = 0;
	QString reason;
};

struct HttpPacket{
	RequestLine request;
	ResponseLine response;
	QString contType;
	unsigned long int contLen;
	QByteArray body;
};

class Wget : public QObject
{
	Q_OBJECT
public:
	explicit Wget(QObject *parent = 0);
	bool isRunning() { return m_running; }
	void get(QString url);
	QByteArray getBody() { return m_packet.body; }
signals:
	void signal_complete();
private slots:
	void slot_readyRead();
private:
	QTcpSocket* m_pSocket;
	bool m_running = false;
	HttpPacket m_packet;
	QByteArray m_buffer;

	void parsHead(const QByteArray &data);
	void clearData();
};

#endif // WGET_H
