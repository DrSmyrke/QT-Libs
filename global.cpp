#include "myfunctions.h"
#include "global.h"

#include <QDataStream>
#include <QIODevice>
#include <QCryptographicHash>
#include <QProcess>
#include <QDateTime>

namespace mf {
	Config conf;
	
	void loadSettings()
	{
		QSettings settings("MySoft","APPNAME");

		//app::conf.maxThreads = settings.value("SERVER/maxThreads",app::conf.maxThreads).toUInt();
		//app::conf.port = settings.value("SERVER/port",app::conf.port).toUInt();
	}

	void saveSettings()
	{
		QSettings settings("MySoft","APPNAME");
		settings.clear();

		//settings.setValue("SERVER/maxThreads",app::conf.maxThreads);
		//settings.setValue("SERVER/port",app::conf.port);
	}

	bool parsArgs(int argc, char *argv[])
	{
		bool ret = true;
		for(int i=0;i<argc;i++){
			if(QString(argv[i]).indexOf("-")==0){
				if(QString(argv[i]) == "--help" or QString(argv[1]) == "-h"){
					printf("Usage: %s [OPTIONS]\n"
							"  -l <FILE>    log file\n"
							"  -v    Verbose output\n"
							"\n", argv[0]);
					ret = false;
				}
				if(QString(argv[i]) == "-l") app::conf.logFile = QString(argv[++i]);
				if(QString(argv[i]) == "-v") app::conf.verbose = true;
			//}else{
			//	bool ok = false;
			//	QString(argv[i]).toInt(&ok,10);
			//	if(ok) app::conf.port = QString(argv[i]).toInt();
			}
		}
		return ret;
	}

	void setLog(const uint8_t logLevel, const QString &mess)
	{
		if(app::conf.logLevel < logLevel or app::conf.logLevel == 0) return;

		QDateTime dt = QDateTime::currentDateTime();
		QString str = dt.toString("yyyy.MM.dd [hh:mm:ss] ") + mess + "\n";

		if( app::conf.verbose ){
			printf( "%s", str.toUtf8().data() );
			fflush( stdout );
		}

		if( app::conf.logFile.isEmpty() ) return;
		FILE* f;
		f = fopen(app::conf.logFile.toUtf8().data(),"a+");
		fwrite(str.toUtf8().data(),static_cast<size_t>(str.length()),1,f);
		fclose(f);
	}
	
}
