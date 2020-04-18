#include "layerwidget.h"

LayerWidget::LayerWidget(QWidget *parent) : QWidget(parent)
{
	m_iconSize = QSize( 64, 64 );
	m_layerNum = 0;
}

void LayerWidget::addItem(const QString &icoFile, const QString &data)
{
	QStringList list;
	list.append( icoFile );
	list.append( data );
	m_data.append( list );

	QPushButton* pb = new QPushButton( QIcon( icoFile ), "", this );
	pb->setIconSize( m_iconSize );
	connect( pb, &QPushButton::clicked, this, [this, data](){
//		QString file = "";
//		for( auto elem:m_data ){

//		}
		emit signal_selectItem( m_layerNum, data );
	} );
	m_buttons.append( pb );

	replaceWidgets();
}

void LayerWidget::replaceWidgets()
{
	int x = 2;
	int y = 2;
	int w = this->width() - 4;
	int h = this->height();
	int rw = w / ( m_iconSize.width() + 4 );
	int rc = 1;

	if( m_buttons.length() == 0 ){
		return;
	}

	for( auto but:m_buttons ){
		but->setGeometry( x, y, m_iconSize.width(), m_iconSize.height() );
		if( rc++ >= rw ){
			rc = 1;
			y += m_iconSize.height() + 2;
			x = 2;
			continue;
		}
		x += m_iconSize.width() + 2;
	}

	if( x > 2 ){
		y += m_iconSize.height();
	}

	this->setFixedHeight( y );
}

void LayerWidget::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED( event )
	replaceWidgets();
}
