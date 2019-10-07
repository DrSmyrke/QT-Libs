#ifndef HTTP_H
#define HTTP_H

#include <QString>
#include <QList>
#include <QMap>

namespace http{
	struct AuthMethod{
		enum{
			Unknown,
			Basic,
			Digest,
			NTLM,
		};
	};

	struct Proto{
		enum{
			UNKNOWN,
			HTTP,
			HTTPS,
		};
	};

	struct AuthData{
		uint8_t method =  http::AuthMethod::Unknown;
		QString methodString;
		QString username;
		QString realm;
		QString nonce;
		QString uri;
		QString nc;
		QString cnonce;
		QString response;
		QString opaque;
		QByteArray BasicString;
		QString qop;
	};

	struct pkt{
		struct {
			struct {
				QString proto = "HTTP/1.1";
				uint16_t code = 0;
				QString comment;
			}response;
			struct {
				QString method;
				QString target;
				QString proto = "HTTP/1.1";
			}request;
			QString userAgent;
			QString proxyConnection;
			QString connection = "close";
			QString host;
			QString contType;
			QString location;
			QString contEncoding;
			int32_t contLen = 0;
			QString cacheControl;
			QString cookie;
			QString reffer;
			QString accept;
			QString acceptEncoding;
			QString acceptLanguage;
			QString proxyAuthenticate;
			QString proxyAuthorization;
			bool valid = false;
			bool flag = true;
			bool firstLine = true;
			uint32_t length;
		}head;
		struct {
			QByteArray rawData;
			bool valid = false;
		}body;
		bool isResponse = false;
		bool isRequest = false;
		bool valid = false;
		bool next = false;
	};

	pkt parsPkt(const QByteArray &data);
	QByteArray buildPkt(http::pkt &pkt);
	AuthData parsAuthString(const QByteArray &data);
	void parsArguments(const QString &string, QMap< QByteArray, QByteArray > &args);
};

#endif // HTTP_H
