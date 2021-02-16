#ifndef LOGGER_H
#define LOGGER_H

#include <QDir>

struct LoggerConfig{
	bool verbose			= false
	uint8_t logLevel		= 3;
#ifdef __linux__
	QString logFile			= "/var/log/app";
#elif _WIN32
	QString logFile			= QDir::homePath() + "/app.log";
#endif
	FILE* file				= 0;
};

namespace Logger {
	extern LoggerConfig conf;

	void setLog(const uint8_t logLevel, const QString &mess);
}

#endif // LOGGER_H
