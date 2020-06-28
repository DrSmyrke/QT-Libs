#include "api.h"

#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace API {

	Response sendRequest(Request &request)
	{
		API::Response response;

		if( !request.url.isValid() ){
			response.errorString = "Url not valid";
			response.error = true;
			return response;
		}

		QNetworkAccessManager manager;
		QNetworkRequest networkRequest( request.url );

		//QString auth = QString( "%1:%2" ).arg( "api" ).arg( "th48h745hy458yh485yh8" );
		//request.setRawHeader( "Authorization", "Basic " + auth.toUtf8().toBase64() );
		networkRequest.setRawHeader( "User-Agent", "Dr.Smyrke API Agent v0.1" );

		if( request.dataList.size() > 0 ){

			QUrlQuery params;
			QJsonObject jsonObj;
			QJsonDocument jsonDoc;

			switch( request.dataType ){
				case API::DataType::JSON:
					for( auto elem:request.dataList ){
						jsonObj.insert( elem.first, elem.second.toString() );
					}
					jsonDoc.setObject( jsonObj );
					request.data = jsonDoc.toJson( QJsonDocument::Compact );
				break;
				case API::DataType::INI: break;
				case API::DataType::URLQUERY:
					for( auto elem:request.dataList ){
						params.addQueryItem( elem.first, elem.second.toString() );
					}
					request.data = params.query().toUtf8();
				break;
			}
		}

		QNetworkReply *reply;

		switch( request.method ){
			case API::Method::GET:
				reply = manager.get( networkRequest );
			break;
			case API::Method::POST:
				networkRequest.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
				reply = manager.post( networkRequest, request.data );
			break;
			case API::Method::PUT:
				reply = manager.put( networkRequest, request.data );
			break;
			//case API::Method::DELETE:
				//reply = manager.( networkRequest, request.data );
			//break;
			default:
				response.errorString = "Method invalid";
				response.error = true;
				return response;
			break;
		}


		QTimer timer;
		timer.setInterval( 10000 );
		timer.setSingleShot( true );
		QEventLoop loop;

		QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
		QObject::connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
		timer.start();
		loop.exec();

		if( reply->isFinished() ){
			response.data = reply->readAll();
			response.code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
			if( reply->error() != QNetworkReply::NoError ){
				response.error = true;
				response.errorString = reply->errorString();
			}
		}else{
			response.error = true;
			response.errorString = "Timeout error";
		}


		reply->deleteLater();

		return response;
	}
	
	QByteArray packData(const std::map<QString, QVariant> &dataList)
	{
		QByteArray ba;
		QJsonObject jsonObj;
		QJsonDocument jsonDoc;

		for( auto elem:dataList ){
			jsonObj.insert( elem.first, elem.second.toString() );
		}
		jsonDoc.setObject( jsonObj );
		ba = jsonDoc.toJson( QJsonDocument::Compact );

		return ba;
	}

}
