#ifndef HEXVIEWER_H
#define HEXVIEWER_H

#include <QWidget>
#include <QLabel>

class HexViewer : public QWidget
{
	Q_OBJECT
public:
	explicit HexViewer(QWidget *parent = nullptr);
	void setBufferSize(uint16_t size){ m_bufferSize = size; }
	void appendData(const QByteArray &data);
	void setBlockSize(const uint8_t size){ m_blockSize = size; }
	void clearData();
signals:
	void signal_customContextMenu(const QPoint &pos);
private:
	uint16_t m_bufferSize;
	QByteArray m_buffer;
	QLabel* m_pHexL;
	QLabel* m_pAsciiL;
	QFont m_font;
	uint8_t m_yStep;
	uint16_t m_yTotal;
	uint16_t m_blockSize;
	uint8_t m_blocksCount;
	uint8_t m_xSteep;
	uint8_t m_xAsciiSteep;
	uint16_t m_hexWidth;
	uint16_t m_asciiWidth;

	void yRecount();
protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
};

#endif // HEXVIEWER_H
