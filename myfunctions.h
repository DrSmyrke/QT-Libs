#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H
#include <QDir>

namespace mf {
	void removeDir(const QString &path);
	QString getSize(const long val);
	QByteArray toBigEndianInt(const int val);
	QByteArray toBigEndianShort(const short val);
	unsigned int getRand(const int max);
	/**
	 * @brief md5
	 * Get md5 summ for string
	 * @param string
	 * @return
	 */
	QByteArray md5(const QByteArray &string);
	bool startDetached(const QString &cmd, const QStringList &args);
	bool checkFile(const char *fileName);
	bool strFind(const QString &inStr, const QString &dataStr);
	QByteArray toLittleEndianShort(const short val);
}

#endif // MYFUNCTIONS_H
