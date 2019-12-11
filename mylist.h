#ifndef MYLIST_H
#define MYLIST_H

#include <QObject>
#include <QAbstractTableModel>
#include <QUrl>

#include "global.h"

class MyList : public QAbstractTableModel
{
	Q_OBJECT
public:
	MyList();
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool addTarget(const QUrl &url);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	void requestConnect(int row);
public slots:
	void slotUpdate();
	void slot_stop();
signals:
	void signal_finished();
private:
	std::vector<Connector*> m_data;
};

#endif // MYLIST_H
