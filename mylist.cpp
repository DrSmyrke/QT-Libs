#include "mylist.h"

MyList::MyList()
{

}

int MyList::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED( parent )
	return m_data.size();
}

int MyList::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED( parent )
	return 2;
}

QVariant MyList::data(const QModelIndex &index, int role) const
{
	QVariant data;
	if( role == Qt::DisplayRole || role == Qt::EditRole ){
		auto elem = m_data.at( index.row() );
		auto state = elem->getState();
		auto stateStr = setColorText( state );
		if( state == StatusConnectState::normal ) stateStr += QString(" listen at [%1]").arg( elem->getLocalPort() );
		if( index.column() == 0 ) data.setValue( elem->getTarget().toString() );
		if( index.column() == 1 ) data.setValue( stateStr );
	}
	return data;
}

bool MyList::addTarget(const QUrl &url)
{
	if( !url.isValid() ) return false;

	this->beginInsertRows( QModelIndex(), m_data.size(), m_data.size() + 1 );
	bool res = false;
	bool find = false;
	for( auto elem:m_data ){
		if( elem->getTarget() == url ){
			find = true;
			break;
		}
	}

	if( !find ){
		app::setLog(3,QString("MyList::addTarget [%1:%2][%3]").arg( url.host() ).arg( url.port() ).arg( url.scheme() ));
		if( !find ){
			Connector* connector = new Connector( url, this );
			connect( this, &MyList::signal_finished, connector, &Connector::slot_stop );
			connect( connector, &Connector::signal_finished, this, [this](Connector* connector){
				for( auto it = m_data.begin(); it != m_data.end(); it++ ){
					if( (*it) == connector ){
						m_data.erase( it );
						break;
					}
				}
				connector->deleteLater();
			} );
			m_data.push_back( connector );
			res = true;
		}
	}

	this->endInsertRows();

	return res;
}

Qt::ItemFlags MyList::flags(const QModelIndex &index) const
{
	auto flags = QAbstractTableModel::flags(index);
	flags &= ~ Qt::ItemIsSelectable;
	if( index.column() == 1 ) return flags;
	return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool MyList::setData(const QModelIndex &index, const QVariant &value, int role)
{
	bool res = false;
	if( role == Qt::EditRole && index.column() == 0 ){
		QUrl url;
		url.setUrl( value.toString() );
		if( url.isValid() ){
			m_data.at( index.row() )->setTarget( url );
			res = true;
		}
	}
	return res;
}

void MyList::requestConnect(int row)
{
	m_data.at( row )->init();
}

void MyList::slotUpdate()
{
	emit(dataChanged(QModelIndex(), QModelIndex()));
}

void MyList::slot_stop()
{
	emit signal_finished();
	m_data.clear();
}
