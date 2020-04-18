#include "imageeditor.h"
#include <QPainter>

ImageEditor::ImageEditor(QWidget *parent) : QWidget(parent)
{
	m_cropMode		= false;

	m_resizeBox.setWidth(20);
	m_resizeBox.setHeight(20);
	this->setMouseTracking(true);
	setCropSize(QSize(50,100));
	setLayersCount( 1 );
}
void ImageEditor::setImage(const QString &file, const uint8_t layerNum)
{
	if( layerNum >= m_layers.size() ) return;
	m_layersOrig[ layerNum ].load( file );
	m_layers[ layerNum ] = m_layersOrig[ layerNum ];

	m_maxSize = QSize( 0, 0 );
}
QImage ImageEditor::getImage()
{
	QImage img;

	for( auto layerImg:m_layers ){
		if( img.isNull() ) img = layerImg;
		QPainter painter(&img);
		painter.drawImage(QPoint(0, 0), layerImg);
		painter.end();
	}

	if( m_cropMode ){
		float rw = ( m_maxSize.width() >= img.width() ) ? (float)m_maxSize.width() / (float)img.width() : (float)img.width() / (float)m_maxSize.width() ;
		float rh = ( m_maxSize.height() >= img.height() ) ? (float)m_maxSize.height() / (float)img.height() : (float)img.height() / (float)m_maxSize.height() ;
		int x = m_ramkaPos.x() - m_ramkaOffset.x();
		int y = m_ramkaPos.y() - m_ramkaOffset.y();
		int w = m_ramkaSize.width() / rw;
		int h = m_ramkaSize.height() / rh;
		x /= rw;
		y /= rh;
		return img.copy( x, y, w, h);
	}

	return img;
}

void ImageEditor::setCropSize(const QSize &size)
{
	m_r = (float)size.width() / (float)size.height();
	m_ramkaSize.setWidth( size.width() );
	m_ramkaSize.setHeight( size.height() );

	m_ramkaPos.setX((this->width()/2)-(m_ramkaSize.width()/2));
	m_ramkaPos.setY((this->height()/2)-(m_ramkaSize.height()/2));
}

void ImageEditor::setLayersCount(const uint8_t count)
{
	m_layers.clear();
	m_layersOrig.clear();
	for( uint8_t i = 0; i < count; i++ ){
		QImage img;
		m_layers.push_back( img );
		m_layersOrig.push_back( img );
	}

	m_maxSize = QSize( 0, 0 );
}

void ImageEditor::setLayerHSV(const uint8_t layerNum, int color, int saturation, int light)
{
	if( layerNum >= m_layers.size() ) return;

	m_layers[ layerNum ] = m_layersOrig[ layerNum ];

	for (int i = 0; i < m_layers[ layerNum ].width(); i++){
		for (int j = 0; j < m_layers[ layerNum ].height(); j++){
			QColor pixelColor = m_layers[ layerNum ].pixelColor(i, j);
			//Если пиксель прозрачный
			if( pixelColor.alpha() == 0 ) continue;
			//Если пиксель уже цветной
			if( pixelColor.hsvSaturation() > 0 ) continue;
			if( pixelColor.red() == 0 && pixelColor.green() == 0 && pixelColor.blue() == 0 ) continue;
			if( pixelColor.red() == 255 && pixelColor.green() == 255 && pixelColor.blue() == 255 ) continue;

			if( pixelColor.lightness() < 5 ) continue;

			//int r = pixelColor.lightness() / 2;
			int s = saturation;
			if( s > 100 ){
				s = 100;
			}
			if( s < 0 ){
				s = 0;
			}
			int h = color;
			if( h > 359 ){
				h = 359;
			}
			if( h < 0 ){
				h = 0;
			}

			//int newBright = color.lightness() + light - r;
			//if( newBright > 100 ) newBright = 100;
			//if( newBright < 0 ) newBright = 0;

			pixelColor.setHsv( h, s, pixelColor.lightness(),  pixelColor.alpha() );
			pixelColor = pixelColor.lighter( light );

			m_layers[ layerNum ].setPixelColor(i, j, pixelColor);
		}
	}
	this->update();
}

void ImageEditor::setCropMode(bool cropMode)
{
	m_cropMode = cropMode;
	this->update();
}

void ImageEditor::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter p(this);

	int rx = 0;
	int ry = 0;
	m_ri = 0;

	for( auto img:m_layers ){
		float ri = ( img.width() >= img.height() ) ? (float)img.height() / (float)img.width() : (float)img.width() / (float)img.height() ;
		float nw = 0;
		float nh = 0;
		if( img.width() >= img.height() ){
			nw = this->width();
			nh = nw * ri;
		}else{
			nh = this->height();
			nw = nh * ri;
		}
		int lrx = 0;
		int lry = 0;

		if( this->width() > img.width() || this->height() > img.height() ){
			lrx = ( this->width() / 2 ) - ( nw / 2 );
			lry = ( this->height() / 2 ) - ( nh / 2 );
		}
//		int iw = img.width();
//		int ih = img.height();
//		int ww = this->width();
//		int wh = this->height();

		if( lrx > rx ){
			rx = lrx;
		}
		if( lry > ry ){
			ry = lry;
		}
		if( nw > m_maxSize.width() ){
			m_maxSize.setWidth( nw );
		}
		if( nh > m_maxSize.height() ){
			m_maxSize.setHeight( nh );
		}

		m_ramkaOffset.setX( lrx );
		m_ramkaOffset.setY( lry );

		QRectF rect;
		rect.setX( lrx );
		rect.setY( lry );
		rect.setWidth( nw );
		rect.setHeight( nh );
		p.drawImage( rect, img );
	}

	if( !m_cropMode ){
		return;
	}

	//чтоб рамка за границы изображения не уползла
	if(m_ramkaPos.x()<rx) m_ramkaPos.setX(rx);
	if(m_ramkaPos.y()<ry) m_ramkaPos.setY(ry);
	if(m_ramkaPos.y()+m_ramkaSize.height()>m_maxSize.height()+ry) m_ramkaPos.setY(m_maxSize.height()-m_ramkaSize.height()+ry);
	if(m_ramkaPos.x()+m_ramkaSize.width()>m_maxSize.width()+rx) m_ramkaPos.setX(m_maxSize.width()-m_ramkaSize.width()+rx);

	//draw shadow
	p.setPen(Qt::NoPen);
	p.setBrush(QBrush(QColor(100,100,100,200)));
	p.drawRect(rx,ry,m_maxSize.width(),m_ramkaPos.y()-ry);
	p.drawRect(rx,m_ramkaPos.y()+m_ramkaSize.height(),m_maxSize.width(),m_maxSize.height()-(m_ramkaPos.y()+m_ramkaSize.height())+ry);
	p.drawRect(rx,m_ramkaPos.y(),m_ramkaPos.x()-rx,m_ramkaSize.height());
	p.drawRect(m_ramkaPos.x()+m_ramkaSize.width(),m_ramkaPos.y(),m_maxSize.width()-(m_ramkaPos.x()+m_ramkaSize.width())+rx,m_ramkaSize.height());

	//draw resize boxes
	int r=255;
	int g=255;
	int b=255;
	int a=100;
	int rBoxW2=m_resizeBox.width()/2;
	int rBoxH2=m_resizeBox.height()/2;
	p.setPen(Qt::NoPen);

	//отрисовка ресайз боксов
	p.setBrush(QBrush(QColor(r,g,b,a)));
	if(m_mx>m_ramkaPos.x()-rBoxW2 and m_mx<m_ramkaPos.x()+rBoxW2 and m_my>m_ramkaPos.y()-rBoxH2 and m_my<m_ramkaPos.y()+rBoxH2){
		p.setBrush(QBrush(QColor(255,147,0,50)));
	}
	p.drawRect(m_ramkaPos.x()-rBoxW2,m_ramkaPos.y()-rBoxH2,m_resizeBox.width(),m_resizeBox.height());

	p.setBrush(QBrush(QColor(r,g,b,a)));
	if(m_mx>m_ramkaPos.x()-rBoxW2 and m_mx<m_ramkaPos.x()+rBoxW2 and m_my>m_ramkaPos.y()+m_ramkaSize.height()-rBoxH2 and m_my<m_ramkaPos.y()+m_ramkaSize.height()+rBoxH2){
		p.setBrush(QBrush(QColor(255,147,0,50)));
	}
	p.drawRect(m_ramkaPos.x()-rBoxW2,m_ramkaPos.y()+m_ramkaSize.height()-rBoxH2,m_resizeBox.width(),m_resizeBox.height());

	p.setBrush(QBrush(QColor(r,g,b,a)));
	if(m_mx>m_ramkaPos.x()+m_ramkaSize.width()-rBoxW2 and m_mx<m_ramkaPos.x()+m_ramkaSize.width()+rBoxW2 and m_my>m_ramkaPos.y()+m_ramkaSize.height()-rBoxH2 and m_my<m_ramkaPos.y()+m_ramkaSize.height()+rBoxH2){
		p.setBrush(QBrush(QColor(255,147,0,50)));
	}
	p.drawRect(m_ramkaPos.x()+m_ramkaSize.width()-rBoxW2,m_ramkaPos.y()+m_ramkaSize.height()-rBoxH2,m_resizeBox.width(),m_resizeBox.height());

	p.setBrush(QBrush(QColor(r,g,b,a)));
	if(m_mx>m_ramkaPos.x()+m_ramkaSize.width()-rBoxW2 and m_mx<m_ramkaPos.x()+m_ramkaSize.width()+rBoxW2 and m_my>m_ramkaPos.y()-rBoxH2 and m_my<m_ramkaPos.y()+rBoxH2){
		p.setBrush(QBrush(QColor(255,147,0,50)));
	}
	p.drawRect(m_ramkaPos.x()+m_ramkaSize.width()-rBoxW2,m_ramkaPos.y()-rBoxH2,m_resizeBox.width(),m_resizeBox.height());

	//draw ramka
	p.setBrush(Qt::NoBrush);
	p.setPen(Qt::white);
	p.drawRect(m_ramkaPos.x(),m_ramkaPos.y(),m_ramkaSize.width(),m_ramkaSize.height());
}
void ImageEditor::mousePressEvent(QMouseEvent *event)
{
	int mx=event->pos().x();
	int my=event->pos().y();
	int rx=m_ramkaPos.x()+m_ramkaSize.width();
	int ry=m_ramkaPos.y()+m_ramkaSize.height();
	if(mx>m_ramkaPos.x() and mx<rx and my>m_ramkaPos.y() and my<ry){
		m_ramkaMove=true;
		m_pressPos=event->pos();
	}
	int rBoxW2=m_resizeBox.width()/2;
	int rBoxH2=m_resizeBox.height()/2;
	if(m_mx>m_ramkaPos.x()-rBoxW2 and m_mx<m_ramkaPos.x()+rBoxW2 and m_my>m_ramkaPos.y()-rBoxH2 and m_my<m_ramkaPos.y()+rBoxH2){
		m_resizeLeftTop=true;
		m_ramkaMove=false;
		m_pressPos=event->pos();
	}
	if(m_mx>m_ramkaPos.x()-rBoxW2 and m_mx<m_ramkaPos.x()+rBoxW2 and m_my>m_ramkaPos.y()+m_ramkaSize.height()-rBoxH2 and m_my<m_ramkaPos.y()+m_ramkaSize.height()+rBoxH2){
		m_resizeLeftBottom=true;
		m_ramkaMove=false;
		m_pressPos=event->pos();
	}
	if(m_mx>m_ramkaPos.x()+m_ramkaSize.width()-rBoxW2 and m_mx<m_ramkaPos.x()+m_ramkaSize.width()+rBoxW2 and m_my>m_ramkaPos.y()+m_ramkaSize.height()-rBoxH2 and m_my<m_ramkaPos.y()+m_ramkaSize.height()+rBoxH2){
		m_resizeRightBottom=true;
		m_ramkaMove=false;
		m_pressPos=event->pos();
	}
	if(m_mx>m_ramkaPos.x()+m_ramkaSize.width()-rBoxW2 and m_mx<m_ramkaPos.x()+m_ramkaSize.width()+rBoxW2 and m_my>m_ramkaPos.y()-rBoxH2 and m_my<m_ramkaPos.y()+rBoxH2){
		m_resizeRightTop=true;
		m_ramkaMove=false;
		m_pressPos=event->pos();
	}
	QWidget::mousePressEvent(event);
}
void ImageEditor::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	m_ramkaMove=false;
	m_resizeLeftTop=false;
	m_resizeLeftBottom=false;
	m_resizeRightBottom=false;
	m_resizeRightTop=false;
}
void ImageEditor::mouseMoveEvent(QMouseEvent *event)
{
	m_mx=event->pos().x();
	m_my=event->pos().y();
	QPoint r(m_pressPos-event->pos());
	if(m_ramkaMove){
		m_ramkaPos-=r;
		m_pressPos=event->pos();

	}
	if(m_resizeLeftTop){
		int tmpW=m_ramkaSize.width();
		int tmpH=m_ramkaSize.height();
		m_ramkaSize.setWidth(m_ramkaSize.width()+r.y());
		chkResise();
		tmpW-=m_ramkaSize.width();
		tmpH-=m_ramkaSize.height();
		m_ramkaPos.setX(m_ramkaPos.x()+tmpW);
		m_ramkaPos.setY(m_ramkaPos.y()+tmpH);
		m_pressPos=event->pos();
	}
	if(m_resizeLeftBottom){
		int tmpW=m_ramkaSize.width();
		m_ramkaSize.setWidth(m_ramkaSize.width()+r.x());
		chkResise();
		tmpW-=m_ramkaSize.width();
		m_ramkaPos.setX(m_ramkaPos.x()+tmpW);
		m_pressPos=event->pos();
	}
	if(m_resizeRightTop){
		int tmp=m_ramkaSize.height();
		m_ramkaSize.setWidth(m_ramkaSize.width()-r.x());
		chkResise();
		tmp-=m_ramkaSize.height();
		m_ramkaPos.setY(m_ramkaPos.y()+tmp);
		m_pressPos=event->pos();
	}
	if(m_resizeRightBottom){
		m_ramkaSize.setWidth(m_ramkaSize.width()-r.x());
		chkResise();
		m_pressPos=event->pos();
	}
	update();
	QWidget::mouseMoveEvent(event);
}
void ImageEditor::chkResise()
{
	if( m_ramkaSize.width() < 25 ){
		m_ramkaSize.setWidth(25);
	}
	if( m_ramkaSize.width() > m_maxSize.width() ){
		m_ramkaSize.setWidth( m_maxSize.width() );
	}
	m_ramkaSize.setHeight( m_ramkaSize.width() / m_r );
	if( m_ramkaSize.height() > m_maxSize.height() ){
		m_ramkaSize.setHeight( m_maxSize.height() );
		m_ramkaSize.setWidth( m_ramkaSize.height() * m_r );
	}
}
