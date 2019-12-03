#include "wget.h"
#include <QHostInfo>

Wget::Wget(QObject *parent) : QObject(parent)
{
	m_pSocket = new QTcpSocket(this);
	connect(m_pSocket,&QTcpSocket::stateChanged,this,[this](const QAbstractSocket::SocketState state){
		if( state == QAbstractSocket::UnconnectedState ) {
			if( m_buffer.size() > 0 ) parsHead(m_buffer);
			m_buffer.clear();
			m_running = false;
			emit signal_complete();
		}
	});
	connect(m_pSocket,&QTcpSocket::readyRead,this,&Wget::slot_readyRead);
}

void Wget::get(QString url)
{
	QString addr;
	QByteArray path;
	uint16_t port = 80;
	QByteArray buff;

	if( url.indexOf("http://",Qt::CaseInsensitive) == 0 ) url.remove(0,7);

	for(uint32_t i = 0; i < (unsigned)url.size(); i++){
		if(addr.isEmpty()){
			if( url[i] == ':' ){
				port = 0;
				addr = buff;
				buff.clear();
				continue;
			}
			if( url[i] == '/' ){
				addr = buff;
				buff.clear();
			}
		}else{
			if( url[i] == '/' and port == 0 ){
				port = buff.toUInt();
				buff.clear();
			}
		}
		buff.append( url.at(i) );
	}
	path.append( buff );

	QHostInfo info = QHostInfo::fromName( addr );
	for(auto elem:info.addresses()){
		m_pSocket->connectToHost( elem, port );
		m_pSocket->waitForConnected(1000);
		if( m_pSocket->isOpen() ) break;
	}

	if( m_pSocket->isOpen() ){
		QByteArray ba;
		ba.append("GET " + path + " HTTP/1.1\r\nHost: " + addr + ":" + QString::number(port) + "\r\nConnection: close\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:59.0) Gecko/20100101 Firefox/59.0\r\n\r\n");
		m_pSocket->write(ba);
		m_pSocket->waitForBytesWritten(100);
		m_running = true;
	}else{
		emit signal_complete();
	}
}
void Wget::slot_readyRead()
{
	while(m_pSocket->bytesAvailable()) m_buffer.append( m_pSocket->read(1024) );
}

void Wget::parsHead(const QByteArray &data)
{
	clearData();
	QByteArray buffF1;
	QByteArray buffF2;
	QByteArray buff;
	QString param;
	bool first = true;
	bool headF = true;
	for(unsigned int i=0;i<(unsigned)data.length();i++){
		if(data[i] == '\r') continue;
		if(param.isEmpty() and data[i] == ':' and data[i+1] == ' '){
			param = buff;
			buff.clear();
			i++;
			continue;
		}
		if(first and data[i] == ' ' and ( buffF1.size() == 0 or buffF2.size() == 0) ){
			if(buffF1.size() == 0){
				buffF1.append(buff);
			}else{
				buffF2.append(buff);
			}
			buff.clear();
			continue;
		}
		if(data[i] == '\n'){
			if(first){
				bool ok;
				buffF2.toInt(&ok);
				if(ok){
					m_packet.response.proto = buffF1;
					m_packet.response.code = buffF2.toInt();
					m_packet.response.reason = buff;
				}else{
					m_packet.request.type = buffF1;
					m_packet.request.url = buffF2;
					m_packet.request.proto = buff;
				}
				buffF1.clear();
				buffF2.clear();
				first = false;
				continue;
			}
			if(headF and param.isEmpty()){
				headF = false;
				continue;
			}
			if(headF){
				if(param.contains("content-length",Qt::CaseInsensitive)) m_packet.contLen = buff.toUInt();
				if(param.contains("content-type",Qt::CaseInsensitive)) m_packet.contType = buff;
				buff.clear();
				param.clear();
				continue;
			}

		}
		buff.append(data.at(i));
	}
	m_packet.body = buff;
	//if(head.contType == "application/sdp") head.sdp = parsSDP(buff);
}

void Wget::clearData()
{
	m_packet.body.clear();
	m_packet.contType.clear();
}
