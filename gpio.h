#ifndef GPIO_H
#define GPIO_H

#include <QObject>
#include "global.h"
#include "myfunctions.h"

class GPIO : public QObject
{
	Q_OBJECT
public:
	enum{
		LOW,
		HIGH,
		INPUT,
		OUTPUT,
	};
	explicit GPIO(QObject *parent = nullptr);
	bool init(uint8_t pinNum, const QString &gpioDir);
	bool deinit();
	bool setDirection( uint8_t direction );
	bool setValue( uint8_t value );
	uint8_t getValue();
	uint8_t chkBTNState();
private:
	QString m_exportFile;
	QString m_unexportFile;
	QString m_directionFile;
	QString m_valueFile;
	uint8_t m_pinNum;
	uint16_t m_pressCounter;
};

#endif // GPIO_H
