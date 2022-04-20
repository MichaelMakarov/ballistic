#include <viewmodel.h>

orbit_observation_model::orbit_observation_model(QObject* const parent) : QAbstractTableModel(parent)
{

}

void orbit_observation_model::update_data(orbit_observation_iter beg, orbit_observation_iter end)
{
	_beg = beg;
	_end = end;
}

int orbit_observation_model::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	return static_cast<int>(std::distance(_beg, _end));
}

int orbit_observation_model::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	return 0;
}

QVariant orbit_observation_model::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
	throw std::logic_error("The method or operation is not implemented.");
}
