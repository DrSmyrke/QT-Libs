#include "hexviewer.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFontDatabase>
#include <QLayout>

HexViewer::HexViewer(QWidget *parent)
	: QWidget(parent)
	, m_bufferSize(65535)
	, m_blockSize( 8 )
{
	//setAttribute( Qt::WA_StaticContents );
	//setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
	//qDebug()<<this->sizePolicy();

	int id = QFontDatabase::addApplicationFont("://Terminus.ttf");
	QString family = QFontDatabase::applicationFontFamilies(id).at(0);

	m_font.setFamily( family );
	m_font.setPointSize( 12 );
	m_font.setWeight( QFont::Normal );
	m_yStep			= 15;
	m_xSteep		= 20;
	m_xAsciiSteep	= 10;

	m_pHexL = new QLabel( this );
		m_pHexL->setWordWrap( true );
	m_pAsciiL = new QLabel( this );
		m_pAsciiL->setWordWrap( true );

	QVBoxLayout* vBox = new QVBoxLayout();
		vBox->addWidget( m_pHexL );
		vBox->addWidget( m_pAsciiL );
	this->setLayout( vBox );
}

void HexViewer::appendData(const QByteArray &data)
{
	m_buffer.append( data );

	yRecount();

	if( m_buffer.size() > m_bufferSize ){
		uint16_t r = m_blocksCount * m_blockSize;
		m_buffer.remove( 0, r );
		yRecount();
	}

	this->update();
	this->setMinimumHeight( m_yTotal );
}

void HexViewer::clearData()
{
	m_buffer.clear();
	yRecount();
	this->update();
	this->setMinimumHeight( m_yTotal );
}

void HexViewer::yRecount()
{
	uint16_t width = static_cast<uint16_t>( this->width() );
	m_hexWidth = ( m_xSteep *  m_blockSize ) + 10;
	m_asciiWidth = ( m_xAsciiSteep *  m_blockSize ) + 5;
	m_blocksCount = width / ( m_hexWidth + m_asciiWidth );
	m_yTotal = static_cast<uint16_t>( m_buffer.size() ) / ( m_blocksCount * m_blockSize );
	m_yTotal += 2;
	m_yTotal *= m_yStep;
}

void HexViewer::paintEvent(QPaintEvent *event)
{
	QRect area = event->rect();
	//qDebug()<<"HexViewer::paintEvent"<<area<<event->region();
	uint16_t asciiX = static_cast<uint16_t>( area.width() ) - ( m_asciiWidth * m_blocksCount );
	QPainter p(this);
	QPen pen;
	pen.setColor(Qt::lightGray);
	p.setPen(pen);
	p.drawLine( asciiX, area.y(), asciiX, area.height() );
	p.setFont( m_font );

	QPen penHText(QColor("#000000"));
	p.setPen(penHText);


	uint16_t x = 5;
	uint16_t ax = 5;
	uint16_t y = m_yStep;
	//y += ( static_cast<uint16_t>( area.y() ) / m_yStep );
	uint8_t ln = 1;
	uint8_t bc = 0;
	//uint16_t iOffset = y / m_yStep;

	//for( uint16_t i = iOffset; i < m_buffer.size(); i++ ){
	for( uint16_t i = 0; i < m_buffer.size(); i++ ){
		auto sym = m_buffer.mid( i, 1 );
		p.drawText( x, y, QString(sym.toHex()) );
		p.drawText( asciiX + ax, y, QString(sym) );

		x += m_xSteep;
		ax += m_xAsciiSteep;

		if( ln == m_blockSize ){
			x += 10;
			ax += 5;
			ln = 0;
			bc++;
		}

		if( bc >= m_blocksCount ){
			y += m_yStep;
			x = 5;
			ax = 5;
			bc = 0;
		}

		ln++;
	}

	//p.drawRect( area );
}

void HexViewer::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED( event )
	yRecount();
	this->setMinimumHeight( m_yTotal );
}

void HexViewer::contextMenuEvent(QContextMenuEvent *event)
{
	signal_customContextMenu( event->pos() );
}
