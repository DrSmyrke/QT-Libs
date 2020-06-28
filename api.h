#ifndef API_H
#define API_H

#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace API {
	struct Method{
		enum{
			GET,
			POST,
			PUT,
			DELETE,
		};
	};
	struct DataType{
		enum{
			JSON,
			INI,
			URLQUERY,
		};
	};
	struct Request{
		QUrl url;
		QByteArray data;
		uint8_t method = API::Method::GET;
		uint8_t dataType = API::DataType::JSON;
		std::map< QString, QVariant > dataList;
	};
	struct Response{
		uint16_t code = 0;
		QByteArray data;
		QString errorString;
		bool error = false;
	};

	Response sendRequest(Request &request);
	QByteArray packData(const std::map< QString, QVariant > &dataList);
}

#endif // API_H
