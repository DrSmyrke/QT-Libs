#include "controlserver.h"

#include <QUrl>

namespace ControlServerResoucres{
	HtmlPage pages;
}

ControlServer::ControlServer(QObject *parent) : QTcpServer(parent)
{
	app::setLog( 0, QString("CONTROL SERVER CREATING ...") );

	app::loadResources( ":/assets/top.html", ControlServerResoucres::pages.top );
	app::loadResources( ":/assets/bottom.html", ControlServerResoucres::pages.bottom );
	app::loadResources( ":/assets/menu.html", ControlServerResoucres::pages.menu );
	app::loadResources( ":/assets/index.html", ControlServerResoucres::pages.index );
	app::loadResources( ":/assets/state.html", ControlServerResoucres::pages.state );
	app::loadResources( ":/assets/admin.html", ControlServerResoucres::pages.admin );

	app::loadResources( ":/assets/buttons.css", ControlServerResoucres::pages.buttonsCSS );
	app::loadResources( ":/assets/color.css", ControlServerResoucres::pages.colorCSS );
	app::loadResources( ":/assets/index.css", ControlServerResoucres::pages.indexCSS );

	app::loadResources( ":/assets/index.js", ControlServerResoucres::pages.indexJS );

	app::loadResources( ":/assets/down-arrow.png", ControlServerResoucres::pages.downArrowIMG );
	app::loadResources( ":/assets/up-arrow.png", ControlServerResoucres::pages.upArrowIMG );
}

ControlServer::~ControlServer()
{
	app::setLog( 0, "CONTROL SERVER DIE..." );
	emit signal_stopAll();
}

bool ControlServer::run()
{
	if(!this->listen( QHostAddress::LocalHost, app::conf.controlPort )){
		app::setLog( 0, QString("CONTROL SERVER [ NOT ACTIVATED ] PORT: [%1] %2").arg( app::conf.controlPort ).arg( this->errorString() ) );
		return false;
	}

	app::setLog( 0, QString("CONTROL SERVER [ ACTIVATED ] PORT: [%1]").arg( app::conf.controlPort ) );

	return true;
}

void ControlServer::stop()
{
	app::setLog( 0, "CONTROL SERVER STOPPING..." );
	app::saveSettings();
	this->close();
}

void ControlServer::incomingConnection(qintptr socketDescriptor)
{
	ControlClient* client = new ControlClient(socketDescriptor);
	//QThread* thread = new QThread();
	//client->moveToThread(thread);
//	connect(thread,&QThread::started,client,&Client::slot_start);
//	connect(client,&Client::signal_finished,thread,&QThread::quit);
	connect(this,&ControlServer::signal_stopAll,client,&ControlClient::slot_stop);
	connect(client,&ControlClient::signal_finished,client,&ControlClient::deleteLater);
//	connect(thread,&QThread::finished,thread,&QThread::deleteLater);
//	thread->start();
	client->run();
}





/// #####################################################################
/// #																	#
/// #						Control Client								#
/// #																	#
/// #####################################################################



ControlClient::ControlClient(qintptr descriptor, QObject *parent)
	: QObject(parent)
	, m_auth(false)
{
	app::setLog(5,QString("ControlClient::ControlClient created"));

	m_pClient = new QTcpSocket();

	if( !m_pClient->setSocketDescriptor( descriptor ) ) slot_stop();

	connect( m_pClient, &QTcpSocket::readyRead, this, &ControlClient::slot_clientReadyRead);
	connect( m_pClient, &QTcpSocket::disconnected, this, [this](){
		//app::setLog(5,QString("ControlClient::ControlClient disconnected"));
		slot_stop();
	});
}

void ControlClient::slot_start()
{

}

void ControlClient::slot_stop()
{
	if( m_pClient->isOpen() ) m_pClient->close();
	emit signal_finished();
}

void ControlClient::slot_clientReadyRead()
{
	QByteArray buff;

	if( !m_pClient->isReadable() ) m_pClient->waitForReadyRead(300);

	while( m_pClient->bytesAvailable() ){
		buff.append( m_pClient->read(1024) );
	}

	app::setLog(5,QString("ControlClient::slot_clientReadyRead %1 bytes [%2] [%3]").arg(buff.size()).arg(QString(buff)).arg(QString(buff.toHex())));

	// Если придет запрос на получение информации
	QByteArray ba;
	if( parsInfoPkt( buff, ba ) ){
		sendToClient(ba);
		return;
	}

	if( !m_auth ){
		//slot_stop();
		//return;
		m_auth = true;
	}

	http::pkt pkt;

	if( m_buff.size() > 0 ){
		m_buff.append( buff );
		pkt = http::parsPkt( m_buff );
	}else{
		pkt = http::parsPkt( buff );
	}
	if( pkt.next ){
		//app::setLog( 5, QString("ControlClient::slot_clientReadyRead packet small") );
		m_buff.append( buff );
		return;
	}

	if( !m_auth ) return;

	if( pkt.valid ){
		processingRequest( pkt );
	}else{
		//app::setLog( 3, QString("ControlClient::slot_clientReadyRead packet is NOT valid [%1][%2][%3]").arg( QString( buff ) ).arg( pkt.head.valid ).arg( pkt.body.valid ) );
		//app::setLog( 3, QString("ControlClient::slot_clientReadyRead packet is NOT valid [%1][%2]").arg( pkt.head.contLen ).arg( pkt.body.rawData.size() ) );
	}
}

void ControlClient::sendToClient(const QByteArray &data)
{
	if( data.size() == 0 ) return;
	if( m_pClient->state() == QAbstractSocket::ConnectingState ) m_pClient->waitForConnected(300);
	if( m_pClient->state() == QAbstractSocket::UnconnectedState ) return;
	m_pClient->write(data);
	m_pClient->waitForBytesWritten(100);
	//app::setLog(5,QString("Client::sendToClient %1 bytes [%2]").arg(data.size()).arg(QString(data.toHex())));
}

bool ControlClient::parsAuthPkt(QByteArray &data)
{
	bool res = false;

	char cmd = data[0];
	data.remove( 0, 1 );

	uint16_t len = 0;
	QByteArray login;
	QByteArray pass;

//	switch( cmd ){
//		case Control::AUTH:
//			len = data[0] << 8;
//			len += data[1];
//			data.remove( 0, 2 );

//			login = data.left( len );
//			data.remove( 0, len );

//			len = data[0] << 8;
//			len += data[1];
//			data.remove( 0, 2 );

//			pass = data.left( len );
//			data.remove( 0, len );

//			app::setLog(5,QString("ControlClient::parsAuthPkt recv [%1:%2]").arg(QString(login)).arg(QString(pass)));

//			if( data[0] == '\0' ) res = true;
//			data.remove( 0, 1 );

//			if( app::chkAuth2( login, pass ) ){
//				m_userLogin = login;
//				m_auth = true;
//				app::setLog(4,QString("ControlClient::parsAuthPkt auth success [%1]").arg(QString(login)));
//			}else{
//				app::setLog(2,QString("ControlClient::parsAuthPkt auth error [%1:%2]").arg(QString(login)).arg(QString(pass)));
//			}

//		break;
//	}

	return res;
}

bool ControlClient::parsInfoPkt(QByteArray &data, QByteArray &sendData)
{
	bool res = false;

	if( data.size() < 2 ) return res;

	char cmd = data[0];
	char param = data[1];

	//if( cmd != Control::INFO ) return res;

	app::setLog(4,QString("ControlClient::parsInfoPkt() [%1]").arg(QString(data.toHex())));

	res = true;
	sendData.clear();
	QString str;
	QStringList list;

//	switch( param ){
//		case Control::VERSION:
//			sendData.append( app::conf.version );
//		break;
//		case Control::USERS:
//			list.clear();
//			for( auto user:app::conf.users ){
//				str = QString("%1	%2	%3	%4	%5	%6	%7").arg( user.login ).arg( app::getUserConnectionsNum( user.login ) ).arg( user.maxConnections ).arg( user.lastLoginTimestamp ).arg( mf::getSize( user.inBytes ) ).arg( mf::getSize( user.outBytes ) ).arg( mf::getSize( user.bytesMax ) );
//				list.push_back( str );
//			}
//			sendData.append( list.join( ";" ) );
//		break;
//		case Control::BLACKLIST_ADDRS:
//			list.clear();
//			for( auto addr:app::accessList.blackDomains ) list.push_back( addr );
//			sendData.append( list.join( ";" ) );
//		break;
//	}

	return res;
}

void ControlClient::sendRawResponse(const uint16_t code, const QString &comment, const QByteArray &data, const QString &mimeType)
{
	http::pkt pkt;
	pkt.head.response.code = code;
	pkt.head.response.comment = comment;
	if( !mimeType.isEmpty() ) pkt.head.contType = mimeType;
	pkt.body.rawData.append( data );
	//app::setLog( 5, QString("ControlClient::sendRawResponse [%1]").arg(pkt.head.response.code) );
	sendToClient( http::buildPkt(pkt) );
}

void ControlClient::sendResponse(const uint16_t code, const QString &comment)
{
	http::pkt pkt;
	pkt.head.response.code = code;
	pkt.head.response.comment = comment;
	pkt.body.rawData.append( getHtmlPage("Service page",comment.toLatin1()) );
	//app::setLog( 5, QString("ControlClient::sendResponse [%1]").arg(pkt.head.response.code) );
	sendToClient( http::buildPkt(pkt) );
}

void ControlClient::processingRequest(const http::pkt &pkt)
{
	bool error = true;
	QString method;

	//app::setLog( 5, QString("ControlClient::processingRequest [%1]").arg( pkt.head.request.target ) );

	if( pkt.head.request.target == "/buttons.css" ){
		sendRawResponse( 200, "OK", ControlServerResoucres::pages.buttonsCSS, "text/css; charset=utf-8" );
		error = false;
	}
	if( pkt.head.request.target == "/color.css" ){
		sendRawResponse( 200, "OK", ControlServerResoucres::pages.colorCSS, "text/css; charset=utf-8" );
		error = false;
	}
	if( pkt.head.request.target == "/index.js" ){
		sendRawResponse( 200, "OK", ControlServerResoucres::pages.indexJS, "application/javascript; charset=utf-8" );
		error = false;
	}
	if( pkt.head.request.target == "/index.css" ){
		sendRawResponse( 200, "OK", ControlServerResoucres::pages.indexCSS, "text/css; charset=utf-8" );
		error = false;
	}
	if( pkt.head.request.target == "/down-arrow.png" ){
		sendRawResponse( 200, "OK", ControlServerResoucres::pages.downArrowIMG, "image/png" );
		return;
	}
	if( pkt.head.request.target == "/up-arrow.png" ){
		sendRawResponse( 200, "OK", ControlServerResoucres::pages.upArrowIMG, "image/png" );
		error = false;
	}
	if( pkt.head.request.target == "/" ){
		sendRawResponse( 200, "OK", getHtmlPage( "Index", ControlServerResoucres::pages.index ), "text/html; charset=utf-8" );
		error = false;
	}
	if( pkt.head.request.target == "/state" ){
		sendRawResponse( 200, "OK", getHtmlPage( "State", ControlServerResoucres::pages.state ), "text/html; charset=utf-8" );
		error = false;
	}
	if( pkt.head.request.target == "/admin" ){
		sendRawResponse( 200, "OK", getHtmlPage( "Admin", ControlServerResoucres::pages.admin ), "text/html; charset=utf-8" );
		error = false;
	}
	if( pkt.head.request.target.indexOf("/get?",Qt::CaseInsensitive) == 0 ) method = "GET";
	if( pkt.head.request.target.indexOf("/set?",Qt::CaseInsensitive) == 0 ) method = "SET";
	if( method == "GET" || method == "SET" ){
		QMap<QByteArray, QByteArray> args;
		http::parsArguments( pkt.head.request.target, args );
		QByteArray response;

		//if( pkt.head.request.method == "GET" ){
		if( method == "GET" ){
			response.append( "content" );
			for( auto param:args.keys() ){
				auto value = args.value( param );
				if( param == "userData" ){
					response.append( QString(":>:%1:>:").arg( QString( param ) ) );

					QString changePass;
					QString maxConnections;
					QString bytesMax;

					changePass += QString("<form class=\"form\" action=\"/set\" onSubmit=\"return changeParam( this, \'alertBoxPass\', true );\">");
					changePass += QString("<input type=\"hidden\" name=\"user\" value=\"%1\">").arg( QString(value) );
					changePass += QString("<input type=\"password\" name=\"newPass\"> <div style=\"display: inline-block;\" id=\"alertBoxPass\"></div>");
					changePass += QString("</form>");

					maxConnections += QString("<form class=\"form\" action=\"/set\" onSubmit=\"return changeParam( this, \'alertBoxMaxConnections\', true );\">");
					maxConnections += QString("<input type=\"hidden\" name=\"user\" value=\"%1\">").arg( QString(value) );
					maxConnections += QString("<input type=\"text\" name=\"newMaxConnections\" value=\"%1\"> <div style=\"display: inline-block;\" id=\"alertBoxMaxConnections\"></div>").arg( 0 );
					maxConnections += QString("</form>");

					bytesMax += QString("<form class=\"form\" action=\"/set\" onSubmit=\"return changeParam( this, \'alertBoxBytesMax\', true );\">");
					bytesMax += QString("<input type=\"hidden\" name=\"user\" value=\"%1\">").arg( QString(value) );
					bytesMax += QString("<input type=\"text\" name=\"newBytesMax\" value=\"%1\"> <div style=\"display: inline-block;\" id=\"alertBoxBytesMax\"></div>").arg( 0 );
					bytesMax += QString("</form>");

					response.append("<table>");
					response.append( QString("<tr><td width=\"120px\">Login:</td><td>%1</td></tr>").arg( "N/A" ) );

					response.append( QString("<tr><td>Password:</td><td>%1</td></tr>").arg( changePass ) );

					response.append( QString("<tr><td>MaxConnections:</td><td>%1</td></tr>").arg( maxConnections ) );
					response.append( QString("<tr><td>bytesMax:</td><td>%1</td></tr>").arg( bytesMax ) );
					response.append("</table>");
				}
				if( param == "serverSettings" ){
					response.append( QString(":>:%1:>:").arg( QString( param ) ) );
					QString logLevel;
					if( m_auth ){
						logLevel += QString("<form class=\"form\" action=\"/set\" onSubmit=\"return changeParam( this, \'alertLogLevel\', true );\">");
						logLevel += QString("<input type=\"hidden\" name=\"sysParam\" value=\"logLevel\">");
						logLevel += QString("<input type=\"number\" name=\"value\" max=\"6\" min=\"0\" value=\"%1\"> <div style=\"display: inline-block;\" id=\"alertLogLevel\"></div>").arg( app::conf.logLevel );
						logLevel += QString("</form>");
					}else{
						logLevel += QString("%1").arg( app::conf.logLevel );
					}

					response.append("<table>");
					response.append( QString("<tr><td>LogLevel:</td><td>%1</td></tr>").arg( logLevel ) );
					//response.append( QString("<tr><td>MaxConnections:</td><td>%1</td></tr>").arg( maxConnections ) );
					//response.append( QString("<tr><td>inBytesMax:</td><td>%1</td></tr>").arg( inBytesMax ) );
					//response.append( QString("<tr><td>outBytesMax:</td><td>%1</td></tr>").arg( outBytesMax ) );
					response.append("</table>");

					continue;
				}
				if( param == "usersData" ){
					response.append( QString(":>:%1:>:").arg( QString( param ) ) );
					response.append("<table>");
					for( auto user:app::conf.users ){
						QString editB = "";

						editB = QString("<input type=\"button\" value=\"EDIT\" onClick=\"edit('user', '%1');\">").arg( user.login );

						//auto str = QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>\n").arg( user.login ).arg( app::getDateTime( user.lastLoginTimestamp ) ).arg( editB );
						//response.append( str );
					}
					response.append("</table>");
					continue;
				}
				if( param == "usersTraffic" ){
					response.append( QString(":>:%1:>:").arg( QString( param ) ) );
					response.append("<table>");
					for( auto user:app::conf.users ){
						auto str = QString("<tr><td>%1</td>%2</tr>\n").arg( user.login ).arg( getUserNetStatsString( user ) );
						response.append( str );
					}
					response.append("</table>");
					continue;
				}
				if( param == "myData" ){
					response.append( QString(":>:%1:>:").arg( QString( param ) ) );
					//auto title = QString("Hi %1 [%2] v%3<br>\n").arg( m_userLogin ).arg( app::getUserGroupNameFromID( myData.group ) ).arg( app::conf.version );
					//response.append( title );
					continue;
				}
				if( param == "myConnections" ){
					response.append( QString(":>:%1:>:").arg( QString( param ) ) );
					response.append("<table>");

//					for( auto elem:app::conf.usersConnections[myData.login] ){
//						auto str = QString("<tr><td>%1</td></tr>\n").arg( elem );
//						response.append( str );
//					}

					response.append("</table>");
					continue;
				}
				if( param == "globalBlockedDomains" ){
					response.append( QString(":>:%1:>:").arg( QString( param ) ) );
					getGlobalBlockedDomains( response, QString( param ) );
					continue;
				}
				if( param == "globalLog" ){
					response.append( QString(":>:%1:>:").arg( QString( param ) ) );
					QFile file;

					file.setFileName( app::conf.logFile );
					if(file.open(QIODevice::ReadOnly)){
						while (!file.atEnd()) response.append( file.readAll() );
						file.close();
					}
					response.replace('\n',"<br>");

					continue;
				}
			}
		}
		if( method == "SET" ){
			if( args.contains( "accessList" ) && args.contains( "value" ) && m_auth ){
				auto param = args.value( "accessList" );
				auto value = args.value( "value" );
				auto updId = args.value( "updId" );
				bool find = false;

//				if( param == "newBlockDomain" ){
//					app::addGlobalBlackAddr( value );
//					getGlobalBlockedDomains( response, updId, myData.group );
//					find = true;
//				}
//				if( param == "deleteBlockDomain" ){
//					app::removeGlobalBlackAddr( value );
//					getGlobalBlockedDomains( response, updId, myData.group );
//					find = true;
//				}

				if( !find ) response.append( "<span class=\"message\">ERROR</span>" );
			}
			if( args.contains( "sysParam" ) && args.contains( "value" ) && m_auth ){
				auto param = args.value( "sysParam" );
				auto value = args.value( "value" );
				bool find = false;

				if( param == "logLevel" ){
					uint8_t level = static_cast<uint8_t>( value.toUShort() );
					if( level > 6 ) level = 6;
					app::conf.logLevel = level;
					app::conf.settingsSave = true;
					find = true;
				}

				if( find ){
					response.append( "<span class=\"valgreen\">Success!</span>" );
				}else{
					response.append( "<span class=\"message\">ERROR</span>" );
				}
			}
			if( args.contains( "newPass" ) && args.contains( "user" ) ){
				auto user = args.value( "user" );
				auto newPass = args.value( "newPass" );
				if( m_auth ){
//					if( app::changePassword( user, newPass ) ){
//						response.append( "<span class=\"valgreen\">Success!</span>" );
//					}else{
//						response.append( "<span class=\"message\">ERROR</span>" );
//					}
				}else{
					response.append( "<span class=\"message\">ERROR</span>" );
				}
			}

			if( args.contains( "newMaxConnections" ) && args.contains( "user" ) ){
				auto user = args.value( "user" );
				auto newMaxConnections = args.value( "newMaxConnections" );
				if( m_auth ){
//					if( app::changeMaxConnections( user, newMaxConnections.toUInt() ) ){
//						response.append( "<span class=\"valgreen\">Success!</span>" );
//					}else{
//						response.append( "<span class=\"message\">ERROR</span>" );
//					}
				}else{
					response.append( "<span class=\"message\">ERROR</span>" );
				}
			}
		}

		if( response.size() == 0 ){
			sendRawResponse( 404, "Not found", "", "text/html; charset=utf-8" );
		}else{
			sendRawResponse( 200, "OK", response, "text/html; charset=utf-8" );
		}
		error = false;
	}

	if( error ) sendResponse( 502, "<h1>Bad Gateway</h1>" );
}

QString ControlClient::getUserNetStatsString(const User &user)
{
	QString str = QString("<td>%1/%2</td><td><img src=\"/down-arrow.png\" class=\"traffIco\"> %4</td><td><img src=\"/up-arrow.png\" class=\"traffIco\"> %6</td><td>%7</td>")
			//.arg( app::getUserConnectionsNum( user.login ) )
			.arg( "N/A" )
			.arg( user.maxConnections )
			.arg( mf::getSize( user.inBytes ) )
			.arg( mf::getSize( user.outBytes ) )
			.arg( mf::getSize( user.bytesMax ) );
	return str;
}

void ControlClient::getGlobalBlockedDomains(QByteArray &buff, const QString &param)
{
	buff.append("<table>");

//	for( auto elem:app::accessList.blackDomains ){
//		QString editB;
//		if( m_auth ){
//			editB += QString("<form class=\"form\" action=\"/set\" onSubmit=\"return changeParam( this, '%1', false, 'Continue to delete the item?' );\">").arg( param );
//			editB += QString("<input type=\"hidden\" name=\"accessList\" value=\"deleteBlockDomain\">");
//			editB += QString("<input type=\"hidden\" name=\"updId\" value=\"%1\">").arg( param );
//			editB += QString("<input type=\"hidden\" name=\"value\" value=\"%1\">").arg( elem );
//			editB += QString("<input type=\"submit\" value=\"DELETE\">");
//			editB += QString("</form>");
//		}

//		auto str = QString("<tr><td>%1</td><td>%2</td></tr>\n").arg( elem ).arg( editB );
//		buff.append( str );
//	}

	if( m_auth ){
		QString str;
		str += QString("<form class=\"form\" action=\"/set\" onSubmit=\"return changeParam( this, '%1' );\">").arg( param );
		str += QString("<input type=\"hidden\" name=\"accessList\" value=\"newBlockDomain\">");
		str += QString("<input type=\"hidden\" name=\"updId\" value=\"%1\">").arg( param );
		str += QString("<input type=\"text\" name=\"value\" placeholder=\"*.google.com\">");
		str += QString("</form>");
		buff.append( QString("<tr><td colspan=\"2\">%1</td></tr>\n").arg( str ) );
	}

	buff.append("</table>");
}

QByteArray ControlClient::getHtmlPage(const QString &title, const QByteArray &content)
{
	QByteArray ba;
	ba.append( ControlServerResoucres::pages.top );
	ba.append( "		<title>-= " );
	ba.append( title );
	ba.append( " =-</title>\n	</head>\n<body>\n" );
	ba.append( ControlServerResoucres::pages.menu );
	ba.append( content );
	ba.append( ControlServerResoucres::pages.bottom );
	return ba;
}

