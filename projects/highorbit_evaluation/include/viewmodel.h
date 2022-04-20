#pragma once
#include <structure_type.h>

#include <qabstractitemmodel.h>

class orbit_observation_model : public QAbstractTableModel {
public:
	orbit_observation_model(QObject* const parent);

	void update_data(orbit_observation_iter beg, orbit_observation_iter end);

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
	orbit_observation_iter _beg, _end;
};