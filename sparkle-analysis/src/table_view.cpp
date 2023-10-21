#include <table_view.hpp>

#include <qheaderview.h>

namespace {

    class table_data_model : public QAbstractTableModel {
      public:
        table_data_model(std::shared_ptr<table_data_provider> provider, QObject *parent)
            : QAbstractTableModel(parent)
            , _provider{std::move(provider)} {
        }

        void update_content() {
            beginResetModel();
            endResetModel();
            emit dataChanged({}, {});
        }

        int rowCount(const QModelIndex &) const override {
            return static_cast<int>(_provider->get_rows_count());
        }

        int columnCount(const QModelIndex &) const override {
            return static_cast<int>(_provider->get_columns_count());
        }

        QVariant data(const QModelIndex &index, int role) const override {
            QVariant out;
            switch (role) {
            case Qt::ItemDataRole::DisplayRole:
                out = QString::fromStdString(_provider->get_data(index.row(), index.column()));
                break;
            case Qt::ItemDataRole::TextAlignmentRole:
                out = _provider->get_alignment(index.column());
                break;
            }
            return out;
        }

        QVariant headerData(int section, Qt::Orientation orientation, int role) const {
            if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
                return QString::fromStdString(_provider->get_header(section));
            } else {
                return QAbstractTableModel::headerData(section, orientation, role);
            }
        }

      private:
        std::shared_ptr<table_data_provider> _provider;
    };

} // namespace

table_view::table_view(std::shared_ptr<table_data_provider> provider, QWidget *parent)
    : QTableView(parent) {
    verticalHeader()->setVisible(true);
    setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    setModel(new table_data_model(std::move(provider), this));
    connect(model(), &QAbstractTableModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, const QVector<int> &) {
        resize_to_contents();
    });
}

void table_view::update_content() {
    dynamic_cast<table_data_model *>(model())->update_content();
}

void table_view::resize_to_contents() {
    resizeColumnsToContents();
    resizeRowsToContents();
}
