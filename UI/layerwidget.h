#ifndef LAYERWIDGET_H
#define LAYERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QList>

class LayerWidget : public QWidget
{
	Q_OBJECT
public:
	explicit LayerWidget(QWidget *parent = nullptr);
	void addItem(const QString& icoFile, const QString &data);
	void setLayerNum(const uint8_t layerNum){ m_layerNum = layerNum; }
signals:
	void signal_selectItem(const uint8_t layerNum, const QString &file);
private:
	QList< QStringList > m_data;
	QList< QPushButton* > m_buttons;
	QSize m_iconSize;
	uint8_t m_layerNum;

	void replaceWidgets();

	// QWidget interface
protected:
	void resizeEvent(QResizeEvent *event);
};

#endif // LAYERWIDGET_H
