#include <tree_view.hpp>

#include <qheaderview.h>

namespace {

    class tree_data_model : public QAbstractItemModel {
      public:
        tree_data_model(std::shared_ptr<tree_data_provider> provider, QObject *parent)
            : QAbstractItemModel(parent)
            , _provider{std::move(provider)} {
        }

        void update_content() {
            beginResetModel();
            endResetModel();
            emit dataChanged({}, {});
        }

        QModelIndex index(int row, int column, const QModelIndex &parent) const {
            if (!hasIndex(row, column, parent)) {
                return {};
            }
            auto parent_item =
                parent.isValid() ? static_cast<tree_data_content *>(parent.internalPointer()) : _provider->get_root_content();
            auto child_item = parent_item->get_child(static_cast<std::size_t>(row));
            return child_item ? createIndex(row, column, child_item) : QModelIndex{};
        }

        QModelIndex parent(const QModelIndex &child) const {
            if (!child.isValid()) {
                return {};
            }
            auto child_item = static_cast<tree_data_content *>(child.internalPointer());
            auto parent_item = child_item->get_parent();
            if (parent_item == _provider->get_root_content()) {
                return {};
            }
            return createIndex(parent_item->get_parent_index(), 0, parent_item);
        }

        int rowCount(const QModelIndex &parent) const {
            auto parent_item =
                parent.isValid() ? static_cast<tree_data_content *>(parent.internalPointer()) : _provider->get_root_content();
            return parent_item ? static_cast<int>(parent_item->get_children_count()) : 0;
        }

        int columnCount(const QModelIndex &parent) const {
            return static_cast<int>(_provider->get_columns_count());
        }

        QVariant data(const QModelIndex &index, int role) const {
            QVariant out;
            switch (role) {
            case Qt::ItemDataRole::DisplayRole:
                out = get_data_content(index);
                break;
            case Qt::ItemDataRole::TextAlignmentRole:
                out = get_data_alignment(index);
                break;
            }
            return out;
        }

        QVariant headerData(int section, Qt::Orientation orientation, int role) const {
            if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
                return QString::fromStdString(_provider->get_header(static_cast<std::size_t>(section)));
            } else {
                return {};
            }
        }
        Qt::ItemFlags flags(const QModelIndex &index) const {
            if (!index.isValid()) {
                return Qt::NoItemFlags;
            }
            return QAbstractItemModel::flags(index);
        }

      private:
        QVariant get_data_content(QModelIndex const &index) const {
            if (index.isValid()) {
                auto ptr = static_cast<tree_data_content *>(index.internalPointer());
                return QString::fromStdString(ptr->get_content(static_cast<std::size_t>(index.column())));
            }
            return {};
        }

        QVariant get_data_alignment(QModelIndex const &index) const {
            if (index.isValid()) {
                auto ptr = static_cast<tree_data_content *>(index.internalPointer());
                return ptr->get_alignment(static_cast<std::size_t>(index.column()));
            }
            return {};
        }

      private:
        std::shared_ptr<tree_data_provider> _provider;
    };

} // namespace

tree_view::tree_view(std::shared_ptr<tree_data_provider> provider, QWidget * parent) : QTreeView(parent)
{
    header()->setStretchLastSection(false);
    setModel(new tree_data_model(std::move(provider), this));
    connect(model(), &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, const QVector<int> &) {
        resize_to_contents();
    });
    connect(this, &QTreeView::expanded, this, [this](auto) { resize_to_contents(); });
}

void tree_view::update_content() {
    dynamic_cast<tree_data_model *>(model())->update_content();
}

void tree_view::resize_to_contents() {
    for (int i{}; i < model()->columnCount(); ++i) {
        resizeColumnToContents(i);
    }
}