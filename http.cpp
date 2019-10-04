#include "http.h"

namespace http{
	pkt parsPkt(const QByteArray &data)
	{
		pkt packet;
		QByteArray miniBuff;
		QString param;

		for(uint32_t i = 0; i < (unsigned)data.size(); i++){
			if( !packet.head.valid && data[i] == '\r') continue;
			if( packet.head.firstLine && !packet.isResponse && !packet.isRequest && data[i] == ' ' ){
				if( miniBuff.contains('/') ){
					packet.head.response.proto = miniBuff;
					packet.isResponse = true;
				}else{
					packet.head.request.method = miniBuff;
					packet.isRequest = true;
				}
				miniBuff.clear();
				continue;
			}
			if( packet.head.firstLine && packet.isRequest && packet.head.request.target.isEmpty() && data[i] == ' ' ){
				packet.head.request.target = miniBuff;
				miniBuff.clear();
				continue;
			}
			if( packet.head.firstLine && packet.isResponse && packet.head.response.code == 0 && data[i] == ' ' ){
				packet.head.response.code = miniBuff.toUInt( nullptr, 10 );
				miniBuff.clear();
				continue;
			}
			if( packet.head.firstLine && packet.isRequest && !packet.head.request.target.isEmpty() && data[i] == '\n' ){
				packet.head.request.proto = miniBuff;
				packet.head.firstLine = false;
				miniBuff.clear();
				continue;
			}
			if( packet.head.firstLine && packet.isResponse && packet.head.response.code != 0 && data[i] == '\n' ){
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
				if(param.contains("content-encoding",Qt::CaseInsensitive)) packet.head.contEncoding = miniBuff;
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

		if( packet.head.contLen == packet.body.rawData.size() ) packet.body.valid = true;
		if( packet.body.rawData.size() < packet.head.contLen ) packet.next = true;

		if( packet.head.valid && packet.body.valid ) packet.valid = true;

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
		if( !pkt.head.location.isEmpty() ) ba.append( "Location: " + pkt.head.location + "\r\n" );

		ba.append( "Content-Length: " + QString::number(pkt.head.contLen) + "\r\n" );
		ba.append( "Connection: " + pkt.head.connection + "\r\n" );
		ba.append( "\r\n" );
		if( pkt.head.contLen > 0 ) ba.append( pkt.body.rawData );
		ba.append( "\r\n" );

		return ba;
	}

	AuthData parsAuthString(const QByteArray &data)
	{
		AuthData authData;
		QByteArray miniBuff;
		QString param;
		bool kavF = false;

		for(uint32_t i = 0; i < (unsigned)data.size(); i++){
			if( data[i] == '\r' || data[i] == '\n' ) continue;

			if( authData.method == http::AuthMethod::Unknown && data[i] == ' ' ){
				param = miniBuff;
				if(param.contains("Basic",Qt::CaseInsensitive)) authData.method = http::AuthMethod::Basic;
				if(param.contains("Digest",Qt::CaseInsensitive)) authData.method = http::AuthMethod::Digest;
				if(param.contains("NTLM",Qt::CaseInsensitive)) authData.method = http::AuthMethod::NTLM;
				authData.methodString = param;
				miniBuff.clear();
				param.clear();
				continue;
			}

			if( param.isEmpty() && data[i] == '=' ){
				param = miniBuff;
				param.replace(" ","");
				miniBuff.clear();
				continue;
			}

			if( data[i] == '"' && miniBuff.size() == 0 ){
				kavF = true;
				continue;
			}
			if( (data[i] == '"' && miniBuff.size() > 0 && kavF) || data[i] == ',' ){
				if(param.contains("realm",Qt::CaseInsensitive))			authData.realm = miniBuff;
				if(param.indexOf("nonce",0,Qt::CaseInsensitive) == 0)		authData.nonce = miniBuff;
				if(param.contains("uri",Qt::CaseInsensitive))			authData.uri = miniBuff;
				if(param.indexOf("nc",0,Qt::CaseInsensitive) == 0)		authData.nc = miniBuff;
				if(param.indexOf("cnonce",0,Qt::CaseInsensitive) == 0)	authData.cnonce = miniBuff;
				if(param.contains("response",Qt::CaseInsensitive))		authData.response = miniBuff;
				if(param.contains("opaque",Qt::CaseInsensitive))		authData.opaque = miniBuff;
				if(param.contains("username",Qt::CaseInsensitive))		authData.username = miniBuff;
				if(param.contains("qop",Qt::CaseInsensitive))			authData.qop = miniBuff;
				param.clear();
				miniBuff.clear();
				continue;
			}

			miniBuff.append( data.at(i) );
		}

		if( authData.method == http::AuthMethod::Basic && miniBuff.size() > 0 ) authData.BasicString = miniBuff;

		return authData;
	}

	void parsArguments(const QString &string, QMap<QByteArray, QByteArray> &args)
	{
		QByteArray method;
		QByteArray tempBuff;
		QByteArray param;
		bool value = false;

		for( uint16_t i = 0; i < string.length(); i++ ){
			if( i == 0 && string[i] == '/' ) continue;
			if( i > 0 ){
				if( string[i] == '?' || string[i] == '=' || string[i] == '&' ){
					if( tempBuff.size() == 0 ) break;
				}
			}
			if( i > 0 && string[i] == '?' && method.size() == 0 && !value ){
				method.append( tempBuff );
				tempBuff.clear();
				continue;
			}

			if( i > 0 && string[i] == '=' && !value ){
				param.append( tempBuff );
				tempBuff.clear();
				continue;
			}

			if( i > 0 && string[i] == '"' && param.size() > 0 ){
				value = !value;
				continue;
			}

			if( i > 0 && string[i] == '&' && !value ){
				if( param.size() == 0 ) break;
				args[param] = tempBuff;
				tempBuff.clear();
				param.clear();
				continue;
			}

			tempBuff.append( string[i] );
		}

		if( param.size() > 0 && tempBuff.size() > 0 ) args[param].push_back( tempBuff );
	}

};
