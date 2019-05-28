#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

struct Config{
	bool verbose						= false;
	uint8_t logLevel					= 3;
	QString logFile						= "/tmp/APPNAME.log";
};

namespace app {
	extern Config conf;

	void loadSettings();
	void saveSettings();
	bool parsArgs(int argc, char *argv[]);
	void setLog(const uint8_t logLevel, const QString &mess);
}

#endif // GLOBAL_H
