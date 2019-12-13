#ifndef CONTROLSERVER_H
#define CONTROLSERVER_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QTcpSocket>

#include "global.h"
#include "myfunctions.h"
#include "http.h"

struct HtmlPage{
	QByteArray top;
	QByteArray bottom;
	QByteArray menu;
	QByteArray indexJS;
	QByteArray colorCSS;
	QByteArray buttonsCSS;
	QByteArray indexCSS;
	QByteArray admin;
	QByteArray state;
	QByteArray index;
	QByteArray downArrowIMG;
	QByteArray upArrowIMG;
};

namespace ControlServerResoucres{
	extern HtmlPage pages;
}


class ControlServer : public QTcpServer
{
	Q_OBJECT
public:
	ControlServer(QObject *parent = nullptr);
	~ControlServer();
	bool run();
	void stop();
protected:
	void incomingConnection(qintptr socketDescriptor);
signals:
	void signal_stopAll();
};

class ControlClient : public QObject
{
	Q_OBJECT
public:
	explicit ControlClient(qintptr descriptor, QObject *parent = 0);
	void run() { slot_start(); }
signals:
	void signal_finished();
public slots:
	void slot_start();
	void slot_stop();
private slots:
	void slot_clientReadyRead();
private:
	QTcpSocket* m_pClient;
	bool m_auth;
	QByteArray m_buff;

	void sendToClient(const QByteArray &data);
	bool parsAuthPkt(QByteArray &data);
	bool parsInfoPkt(QByteArray &data, QByteArray &sendData);
	void sendRawResponse(const uint16_t code, const QString &comment, const QByteArray &data, const QString &mimeType);
	void sendResponse(const uint16_t code, const QString &comment);
	void processingRequest(const http::pkt &pkt);
	QString getUserNetStatsString(const User &user);
	void getGlobalBlockedDomains(QByteArray &buff, const QString &param);

	QByteArray getHtmlPage(const QString &title, const QByteArray &content);

};

#endif // CONTROLSERVER_H
