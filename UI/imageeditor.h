#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <QWidget>
#include <QMouseEvent>

class ImageEditor : public QWidget
{
	Q_OBJECT
public:
	explicit ImageEditor(QWidget *parent = 0);
	void setImage(const QString &file, const uint8_t layerNum = 0);
	QImage getImage();
	void setCropSize(const QSize &size);
	void setLayersCount(const uint8_t count = 1);
	void setLayerHSV(const uint8_t layerNum = 0, int color = -1, int saturation = 0, int light = 100);
	void setCropMode(bool cropMode = false);
protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
private:
	bool m_cropMode;
	QList< QImage > m_layers;
	QList< QImage > m_layersOrig;
	QSize m_maxSize;
	float m_ri;
	QPoint m_ramkaOffset;



	QSize m_ramkaSize;
	QPoint m_ramkaPos;
	bool m_ramkaMove=false;
	QPoint m_pressPos;
	QSize m_resizeBox;
	int m_mx;
	int m_my;
	bool m_resizeLeftTop=false;
	bool m_resizeLeftBottom=false;
	bool m_resizeRightBottom=false;
	bool m_resizeRightTop=false;
	float m_r;



	void chkResise();
};

#endif // IMAGEEDITOR_H
