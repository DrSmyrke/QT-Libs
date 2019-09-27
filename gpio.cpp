#include "gpio.h"

GPIO::GPIO(QObject *parent)
	: QObject(parent)
	, m_pressCounter(0)
{

}

bool GPIO::init(uint8_t pinNum, const QString &gpioDir)
{
	bool res = false;

	m_pinNum = pinNum;

	m_exportFile = QString("%1/export").arg( gpioDir );
	m_unexportFile = QString("%1/unexport").arg( gpioDir );
	m_directionFile = QString("%1/gpio%2/direction").arg( gpioDir ).arg( pinNum );
	m_valueFile = QString("%1/gpio%2/value").arg( gpioDir ).arg( pinNum );

	if( !mf::checkFile( m_exportFile.toUtf8().data() ) ) return res;

	QFile f;
	f.setFileName( m_exportFile );
	if( f.open( QIODevice::WriteOnly ) ){
		f.write( QString::number( m_pinNum ).toUtf8() );
		res = true;
		f.close();
	}

	app::setLog( 3, QString("GPIO [%1] init [%2] ...").arg( m_pinNum ).arg( m_valueFile ) );

	return res;
}

bool GPIO::deinit()
{
	bool res = false;

	app::setLog( 2, "GPIO DEINIT..." );

	if( !mf::checkFile( m_unexportFile.toUtf8().data() ) ) return res;

	QFile f;
	f.setFileName( m_unexportFile );
	if( f.open( QIODevice::WriteOnly ) ){
		f.write( QString::number( m_pinNum ).toUtf8() );
		res = true;
		f.close();
	}

	return res;
}

bool GPIO::setDirection(uint8_t direction)
{
	bool res = false;

	if( !mf::checkFile( m_directionFile.toUtf8().data() ) ) return res;

	QFile f;
	f.setFileName( m_directionFile );
	if( f.open( QIODevice::WriteOnly ) ){

		switch( direction ){
			case GPIO::INPUT:
				f.write( "in" );
				app::setLog( 4, QString("GPIO [%1] SET DIRECTION in").arg( m_pinNum ) );
			break;
			case GPIO::OUTPUT:
				f.write( "out" );
				app::setLog( 4, QString("GPIO [%1] SET DIRECTION out").arg( m_pinNum ) );
			break;
		}

		res = true;
		f.close();
	}

	return res;
}

bool GPIO::setValue(uint8_t value)
{
	bool res = false;

	if( !mf::checkFile( m_directionFile.toUtf8().data() ) ) return res;

	QFile f;
	f.setFileName( m_valueFile );
	if( f.open( QIODevice::WriteOnly ) ){

		switch( value ){
			case GPIO::LOW: f.write( "0" ); break;
			case GPIO::HIGH: f.write( "1" ); break;
		}

		res = true;
		f.close();

		app::setLog( 5, QString("GPIO [%1] SET VALUE [%2]").arg( m_pinNum ).arg( value ) );
	}

	return res;
}

uint8_t GPIO::getValue()
{
	uint8_t res = GPIO::LOW;

	if( !mf::checkFile( m_directionFile.toUtf8().data() ) ) return res;

	QFile f;
	f.setFileName( m_valueFile );
	if( f.open( QIODevice::ReadOnly ) ){
		res = f.read( 1 ).toUShort();
		f.close();
	}

	app::setLog( 5, QString("GPIO [%1] GET VALUE [%2]").arg( m_pinNum ).arg( res ) );

	return res;
}

uint8_t GPIO::chkBTNState()
{
	uint8_t res = 0;

	if( getValue() == 0 ){
		m_pressCounter++;
	}else{
		if( m_pressCounter >= app::conf.gpio.button.pressLongTime ){
			res = 2;
			m_pressCounter = 0;
		}
		if( m_pressCounter >= app::conf.gpio.button.pressShortTime ){
			res = 1;
			m_pressCounter = 0;
		}
		m_pressCounter = 0;
	}

	return res;
}
