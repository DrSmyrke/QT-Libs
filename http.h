#ifndef HTTP_H
#define HTTP_H

#include <QString>

namespace http{
	struct AuthMethod{
		enum{
			Basic,
			Digest,
			NTLM,
		};
	};
	struct Proto{
		enum{
			UNKNOW,
			HTTP,
			HTTPS,
		};
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
			int32_t contLen = 0;
			QString cacheControl;
			QString cookie;
			QString accept;
			QString acceptEncoding;
			QString acceptLanguage;
			QString proxyAuthenticate;
			QString proxyAuthorization;
			bool valid = false;
			bool flag = true;
			bool isResponse = false;
			bool isRequest = false;
			bool firstLine = true;
			uint32_t length;
		}head;
		struct {
			QByteArray rawData;
			bool valid = false;
		}body;
	};

	pkt parsPkt(const QByteArray &data);
	QByteArray buildPkt(http::pkt &pkt);
};

#endif // HTTP_H
