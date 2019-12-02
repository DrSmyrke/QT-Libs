#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QDir>
#include <QHostAddress>

struct Host{
	QHostAddress ip;
	uint16_t port						= 0;
};

struct Config{
	bool verbose						= false;
	uint8_t logLevel					= 3;
#ifdef __linux__
	QString logFile						= "/tmp/APPNAME.log";
#elif _WIN32
	QString logFile						= QDir::homePath() + "/APPNAME.log";
#endif
	QString appName						= "APPNAME";
	QString version						= "0.1";
};

namespace app {
	extern Config conf;

	void loadSettings();
	void saveSettings();
	bool parsArgs(int argc, char *argv[]);
	void setLog(const uint8_t logLevel, const QString &mess);
}

#endif // GLOBAL_H
