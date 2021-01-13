#include "logger.h"
#include <QDateTime>

namespace Logger {
	LoggerConfig conf;

	void setLog(const uint8_t logLevel, const QString &mess)
	{
		if(Logger::conf.logLevel < logLevel or Logger::conf.logLevel == 0) return;

		char* dateStr			= QDateTime::currentDateTime().toString("yyyy.MM.dd [hh:mm:ss]").toUtf8().data();
		char* messType;

		switch( logLevel ){
			case 1:		messType = " CORE ";	break;
			case 2:		messType = " ERROR ";	break;
			case 3:		messType = " INFO ";	break;
			case 4:		messType = " DETAIL INFO ";	break;
			case 5:		messType = " DEBUG INFO ";	break;
		}

		if( Logger::conf.verbose ){
			printf( dateStr );
			printf( messType );
			printf( mess.toUtf8().data() );
			printf( "\n" );
			fflush( stdout );
		}

		if( Logger::conf.logFile.isEmpty() ) return;

		if( !Logger::conf.file ){
			Logger::conf.file = fopen( Logger::conf.logFile.toUtf8().data(), "a+" );
		}

		if( !Logger::conf.file ) return;

		fwrite( dateStr, strlen( dateStr ), 1, Logger::conf.file);
		fwrite( messType, strlen( messType ), 1, Logger::conf.file);
		fwrite( mess.toUtf8().data(), mess.length(), 1, Logger::conf.file);
		fwrite( "\n", 1, 1, Logger::conf.file);

		//FILE* f;
		//f = fopen(app::conf.logFile.toUtf8().data(),"a+");
		//fwrite(str.toUtf8().data(),str.length(),1,f);
		//fclose(f);
	}
}
