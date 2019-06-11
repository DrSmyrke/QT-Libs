#include "http.h"

namespace http{
	pkt parsPkt(const QByteArray &data)
	{
		pkt packet;
		QByteArray miniBuff;
		QString param;

		for(uint32_t i = 0; i < (unsigned)data.size(); i++){
			if( !packet.head.valid && data[i] == '\r') continue;
			if( packet.head.firstLine && !packet.head.isResponse && !packet.head.isRequest && data[i] == ' ' ){
				if( miniBuff.contains('/') ){
					packet.head.response.proto = miniBuff;
					packet.head.isResponse = true;
				}else{
					packet.head.request.method = miniBuff;
					packet.head.isRequest = true;
				}
				miniBuff.clear();
				continue;
			}
			if( packet.head.firstLine && packet.head.isRequest && packet.head.request.target.isEmpty() && data[i] == ' ' ){
				packet.head.request.target = miniBuff;
				miniBuff.clear();
				continue;
			}
			if( packet.head.firstLine && packet.head.isResponse && packet.head.response.code == 0 && data[i] == ' ' ){
				packet.head.response.code = miniBuff.toUInt( nullptr, 10 );
				miniBuff.clear();
				continue;
			}
			if( packet.head.firstLine && packet.head.isRequest && !packet.head.request.target.isEmpty() && data[i] == '\n' ){
				packet.head.request.proto = miniBuff;
				packet.head.firstLine = false;
				miniBuff.clear();
				continue;
			}
			if( packet.head.firstLine && packet.head.isResponse && packet.head.response.code != 0 && data[i] == '\n' ){
				packet.head.response.comment = miniBuff;
				packet.head.firstLine = false;
				miniBuff.clear();
				continue;
			}

			if(  packet.head.flag && data[i] == ' ' && miniBuff.right( 1 ) == ":" ){
				param = miniBuff;
				miniBuff.clear();
				continue;
			}
			if(  packet.head.flag && data[i] == '\n' ){
				if(param == "\n"){
					packet.head.flag = false;
					packet.head.length = i + 1;
					packet.head.valid = true;
				}
				if(param.contains("content-length",Qt::CaseInsensitive)) packet.head.contLen = miniBuff.toUInt();
				if(param.contains("content-type",Qt::CaseInsensitive)) packet.head.contType = miniBuff;
				if(param.contains("user-agent",Qt::CaseInsensitive)) packet.head.userAgent = miniBuff;
				if(param.contains("host",Qt::CaseInsensitive)) packet.head.host = miniBuff;
				if(param.contains("connection",Qt::CaseInsensitive)) packet.head.connection = miniBuff;
				if(param.contains("cookie",Qt::CaseInsensitive)) packet.head.cookie = miniBuff;
				if(param.contains("accept",Qt::CaseInsensitive)) packet.head.accept = miniBuff;
				if(param.contains("cache-control",Qt::CaseInsensitive)) packet.head.cacheControl = miniBuff;
				if(param.contains("accept-encoding",Qt::CaseInsensitive)) packet.head.acceptEncoding = miniBuff;
				if(param.contains("accept-language",Qt::CaseInsensitive)) packet.head.acceptLanguage = miniBuff;
				if(param.contains("proxy-authenticate",Qt::CaseInsensitive)) packet.head.proxyAuthenticate = miniBuff;
				if(param.contains("proxy-authorization",Qt::CaseInsensitive)) packet.head.proxyAuthorization = miniBuff;
				miniBuff.clear();
				param = "\n";
				continue;
			}
			miniBuff.append( data.at(i) );
		}
		packet.body.rawData.append( miniBuff );

		if( packet.head.valid && packet.head.contLen == packet.body.rawData.size() ) packet.body.valid = true;

		return packet;
	}

	QByteArray buildPkt(pkt &pkt)
	{
		pkt.head.contLen = pkt.body.rawData.size();

		QByteArray ba;
		if( pkt.head.response.code > 0 ){
			ba.append( pkt.head.response.proto + " " + QString::number(pkt.head.response.code) + " " + pkt.head.response.comment + "\r\n" );
		}else{
			ba.append( pkt.head.request.method + " " + pkt.head.request.target + " " + pkt.head.request.proto + "\r\n" );
		}
		if( !pkt.head.host.isEmpty() ) ba.append( "Host: " + pkt.head.host + "\r\n" );
		if( !pkt.head.accept.isEmpty() ) ba.append( "Accept: " + pkt.head.accept + "\r\n" );
		if( !pkt.head.proxyAuthenticate.isEmpty() ) ba.append( "Proxy-Authenticate: " + pkt.head.proxyAuthenticate + "\r\n" );
		if( !pkt.head.proxyAuthorization.isEmpty() ) ba.append( "Proxy-Authorization: " + pkt.head.proxyAuthorization + "\r\n" );

		if( !pkt.head.cookie.isEmpty() ) ba.append( "Cookie: " + pkt.head.cookie + "\r\n" );
		if( !pkt.head.userAgent.isEmpty() ) ba.append( "User-Agent: " + pkt.head.userAgent + "\r\n" );
		if( !pkt.head.acceptEncoding.isEmpty() ) ba.append( "Accept-Encoding: " + pkt.head.acceptEncoding + "\r\n" );
		if( !pkt.head.acceptLanguage.isEmpty() ) ba.append( "Accept-Language: " + pkt.head.acceptLanguage + "\r\n" );
		if( !pkt.head.cacheControl.isEmpty() ) ba.append( "Cache-Control: " + pkt.head.cacheControl + "\r\n" );
		if( !pkt.head.contType.isEmpty() ) ba.append( "Content-Type: " + pkt.head.contType + "\r\n" );

		ba.append( "Content-Length: " + QString::number(pkt.head.contLen) + "\r\n" );
		ba.append( "Connection: " + pkt.head.connection + "\r\n" );
		ba.append( "\r\n" );
		if( pkt.head.contLen > 0 ) ba.append( pkt.body.rawData );
		ba.append( "\r\n" );

		return ba;
	}

};
