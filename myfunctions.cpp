#include "myfunctions.h"
#include "global.h"

#include <QDataStream>
#include <QIODevice>
#include <QCryptographicHash>
#include <QProcess>
#include <QDateTime>
#include <QFileInfo>
#include <unistd.h>

namespace mf {

	void removeDir(const QString &path)
	{
		if( !QDir( path ).exists() ) return;
		QDir dir = QDir( path );
		for( auto elem:dir.entryList( QStringList() << "*", QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot ) ){
			if( QDir( path + "/" + elem ).exists() ){
				mf::removeDir( path + "/" + elem );
				continue;
			}
			QFile::remove( path + "/" + elem );
		}
		QDir().rmdir( path );
	}

	QString getSize(const uint32_t val)
	{
		QString str;
		if(val < 1024) str = QString::number(val) + " b";
		if(val >= 1024 && val < 1024000) str = QString::number((float)val/1024.0).left(5) + " Kb";
		if(val >= 1024000 && val < 1024000000) str =  QString::number((float)val/1048576.0).left(5) + " Mb";
		if(val >= 1048576000) str =  QString::number((float)val/1073741824.0).left(5) + " Gb";
		return str;
	}

#if __WORDSIZE == 64
	QString getSize(const uint64_t val)
	{
		QString str;
		if(val < 1024) str = QString::number(val) + " b";
		if(val >= 1024 && val < 1024000) str = QString::number((float)val/1024.0).left(5) + " Kb";
		if(val >= 1024000 && val < 1024000000) str =  QString::number((float)val/1048576.0).left(5) + " Mb";
		if(val >= 1048576000) str =  QString::number((float)val/1073741824.0).left(5) + " Gb";
		return str;
	}
#endif

	QByteArray toBigEndianInt(const int val)
	{
		QByteArray bytes(4, 0x00);
		QDataStream stream(&bytes, QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::BigEndian);
		stream << val;
		bytes.resize(4);
		return bytes;
	}

	QByteArray toBigEndianShort(const short val)
	{
		QByteArray bytes(2, 0x00);
		QDataStream stream(&bytes, QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::BigEndian);
		stream << val;
		bytes.resize(2);
		return bytes;
	}

	unsigned int getRand(const int max)
	{
		return qrand() % max;
	}

	QByteArray md5(const QByteArray &string)
	{
		return QCryptographicHash::hash(string,QCryptographicHash::Md5).toHex();
	}

	bool startDetached(const QString &cmd, const QStringList &args, const QString &workPath)
	{
		QString str = cmd + " " + args.join(" ");
		auto res = QProcess::startDetached( cmd, args, workPath );
		app::setLog(1,QString("[EXEC %1]: %2 in %3").arg( (res)?"true":"false" ).arg(str).arg( workPath ));
		return res;
	}

	bool checkFile(const char *fileName)
	{
		FILE *f;
		f = fopen(fileName,"r");
		if(f == nullptr) return false;
		fclose(f);
		delete f;
		return true;
	}

	bool checkFile(const QString &path)
	{
		return QFileInfo::exists(path) && QFileInfo(path).isFile();
	}

	bool strFind(const QString &inStr, const QString &dataStr)
	{
		bool ret = false;

		if( dataStr.isEmpty() || inStr.isEmpty() ) return ret;
		if( inStr.left(1) == "*" && inStr.right(1) == "*" ){
			QString findStr = inStr;
			findStr.remove( 0, 1 );
			findStr.remove( findStr.length() - 1, 1 );
			if( dataStr.contains( findStr, Qt::CaseInsensitive ) ) ret = true;
		}
		if( inStr.left(1) == "*" && inStr.right(1) != "*" ){
			QString findStr = inStr;
			findStr.remove( 0, 1 );
			if( dataStr.right( findStr.length() ) == findStr ) ret = true;
		}
		if( inStr.left(1) != "*" && inStr.right(1) == "*" ){
			QString findStr = inStr;
			findStr.remove( findStr.length() - 1, 1 );
			if( dataStr.indexOf( findStr, Qt::CaseInsensitive ) == 0 ) ret = true;
		}
		if( inStr.left(1) != "*" && inStr.right(1) != "*" ){ if( dataStr == inStr ) ret = true; }

		return ret;
	}

	QByteArray toLittleEndianShort(const short val)
	{
		QByteArray bytes(2, 0x00);
		QDataStream stream(&bytes, QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << val;
		bytes.resize(2);
		return bytes;
	}

	void XOR(QByteArray &data, const QByteArray &key)
	{
		if( key.size() == 0 ) return;

		for (int i = 0; i < data.size(); i++) {
			data[i] = data[i] ^ key[ i % key.size() ];
		}
	}

        int pidOfProc(const QString &procName)
        {
            int res = -1;

            QDir dir = QDir( "/proc" );
            for( auto elem:dir.entryList( QStringList() << "*", QDir::Dirs | QDir::NoDotAndDotDot ) ){
                    bool test = false;
                    int pid = elem.toInt( &test, 10 );
                    if( !test ) continue;
                    QString cmd = "";

                    QFile file( QString( "/proc/%1/cmdline" ).arg( elem ) );
                    if( file.open( QIODevice::ReadOnly ) ){
                            cmd = file.read( 1024 );
                            QStringList tmp = cmd.split( "/" );
                            cmd = tmp.last();
                            file.close();
                    }

                    if( cmd.isEmpty() || cmd != procName || pid == getpid() ){
                            continue;
                    }

                   res = pid;
                   break;
            }

            return res;
        }
}
