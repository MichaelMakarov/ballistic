#include <datamodel.hpp>
#include <mainmodel.hpp>
#include <gui.hpp>
#include <observation.hpp>
#include <timefmt.hpp>
#include <maths.hpp>
#include <qheaderview.h>
#include <qabstractitemmodel.h>

//------------------------------------------------
//      функции для таблицы
//------------------------------------------------

QString table_header(int col)
{
    switch (col)
    {
    case 0:
        return "T";
    case 1:
        return "X,м";
    case 2:
        return "Y,м";
    case 3:
        return "Z,м";
    case 4:
        return "Vx,м/с";
    case 5:
        return "Vy,м/с";
    case 6:
        return "Vz,м/с";
    default:
        return {};
    }
}

QString number_to_str(double num)
{
    return QString::number(num, 'f', 3);
}

QString data_element(const orbit_data &obs, int col)
{
    switch (col)
    {
    case 0:
        return QString::fromStdString(std::format("{}", obs.t));
    case 1:
        return number_to_str(obs.v[0]);
    case 2:
        return number_to_str(obs.v[1]);
    case 3:
        return number_to_str(obs.v[2]);
    case 4:
        return number_to_str(obs.v[3]);
    case 5:
        return number_to_str(obs.v[4]);
    case 6:
        return number_to_str(obs.v[5]);
    default:
        return {};
    }
}

int alignment(int col)
{
    int alignment = Qt::AlignmentFlag::AlignVCenter;
    switch (col)
    {
    case 0:
        alignment += Qt::AlignmentFlag::AlignLeft;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        alignment += Qt::AlignmentFlag::AlignRight;
        break;
    }
    return alignment;
}

//----------------------------------------------------
//              модель для таблицы
//--------------------------------------------------

class tle_tablemodel : public QAbstractTableModel
{
public:
    tle_tablemodel(const computational_model *model, QObject *parent);

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &, int) const override;
    QVariant headerData(int, Qt::Orientation, int) const;

private:
    const computational_model *_model;
};

tle_tablemodel::tle_tablemodel(const computational_model *model, QObject *parent) : QAbstractTableModel(parent), _model{model}
{
    connect(
        model, &computational_model::tle_data_loaded, this,
        [this]()
        {
            beginResetModel();
            endResetModel();
            emit dataChanged({}, {}, {});
        });
}

QVariant tle_tablemodel::data(const QModelIndex &index, int role) const
{
    QVariant out;
    switch (role)
    {
    case Qt::ItemDataRole::DisplayRole:
        out = data_element(_model->tle_by_index(index.row()), index.column());
        break;
    case Qt::ItemDataRole::TextAlignmentRole:
        out = alignment(index.column());
        break;
    }
    return out;
}

QVariant tle_tablemodel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return table_header(section);
    }
    else
    {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
}

int tle_tablemodel::columnCount(const QModelIndex &) const
{
    return 7;
}

int tle_tablemodel::rowCount(const QModelIndex &) const
{
    return _model->tle_count();
}

//--------------------------------------------------
//              функции для дерева
//--------------------------------------------------

class content_presenter
{
    content_presenter *_parent;
    std::vector<std::unique_ptr<content_presenter>> _children;

public:
    content_presenter(content_presenter *const parent = nullptr, size_t children = {}) : _parent{parent}
    {
        _children.reserve(children);
    }
    void append(content_presenter *const child)
    {
        child->_parent = this;
        _children.push_back(std::unique_ptr<content_presenter>{child});
    }
    void clear() { _children.clear(); }
    int row()
    {
        if (_parent)
        {
            auto begin = std::begin(_parent->_children), end = std::end(_parent->_children);
            auto iter = std::find_if(begin, end, [this](const auto &e)
                                     { return e.get() == this; });
            if (iter != end)
            {
                return static_cast<int>(std::distance(begin, iter));
            }
        }
        return 0;
    }
    content_presenter *child(int row) { return _children[row].get(); }
    content_presenter *parent() { return _parent; }
    int row_count() const { return static_cast<int>(_children.size()); }
    virtual QString content(int index) const { return {}; }
    virtual int alignment(int index) const { return {}; }
};

class seance_presenter : public content_presenter
{
    const observation_seance &_data;

public:
    seance_presenter(const observation_seance &cont, content_presenter *const parent) : content_presenter(parent, cont.m.size()), _data{cont}
    {
    }
    QString content(int index) const override
    {
        switch (index)
        {
        case 0:
            return QString::fromStdString(_data.id);
        case 1:
            return number_to_str(_data.o[0]);
        case 2:
            return number_to_str(_data.o[1]);
        case 3:
            return number_to_str(_data.o[2]);
        default:
            return {};
        }
    }
    int alignment(int index) const override
    {
        return Qt::AlignmentFlag::AlignVCenter + Qt::AlignmentFlag::AlignRight;
    }
};

class measurement_presenter : public content_presenter
{
    const measurement_data &_data;

public:
    measurement_presenter(const measurement_data &cont, content_presenter *const parent) : content_presenter(parent, 0), _data{cont}
    {
    }
    QString content(int index) const override
    {
        switch (index)
        {
        case 4:
            return QString::fromStdString(std::format("{}", _data.t));
        case 5:
            return number_to_str(_data.m);
        case 6:
            return number_to_str(rad_to_deg(_data.i));
        case 7:
            return number_to_str(rad_to_deg(_data.a));
        default:
            return {};
        }
    }
    int alignment(int index) const override
    {
        Qt::AlignmentFlag flag{Qt::AlignmentFlag::AlignVCenter};
        switch (index)
        {
        case 4:
            return flag + Qt::AlignmentFlag::AlignLeft;
        case 5:
        case 6:
        case 7:
            return flag + Qt::AlignmentFlag::AlignRight;
        default:
            return flag;
        }
    }
};

//-------------------------------------------------
//          модель данныз для дерева
//-------------------------------------------------

class measurement_treemodel : public QAbstractItemModel
{
public:
    measurement_treemodel(const computational_model *model, QObject *parent);

    void change_content();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int, Qt::Orientation, int) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    std::unique_ptr<content_presenter> _root;
    const computational_model *_model;
};

measurement_treemodel::measurement_treemodel(const computational_model *model, QObject *parent) : QAbstractItemModel(parent), _model{model}
{
    _root = std::make_unique<content_presenter>();
    connect(model, &computational_model::measurement_data_loaded, this, &measurement_treemodel::change_content);
}

void measurement_treemodel::change_content()
{
    beginResetModel();
    _root->clear();
    int count = _model->seance_count();
    for (int i{}; i < count; ++i)
    {
        auto &seance = _model->seance_by_index(i);
        auto child = new seance_presenter(seance, _root.get());
        for (auto &measurement : seance.m)
        {
            child->append(new measurement_presenter(measurement, child));
        }
        _root->append(child);
    }
    endResetModel();
    emit dataChanged({}, {}, {});
}

QModelIndex measurement_treemodel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return {};
    }
    auto parent_item = parent.isValid() ? static_cast<content_presenter *>(parent.internalPointer()) : _root.get();
    auto child_item = parent_item->child(row);
    return child_item ? createIndex(row, column, child_item) : QModelIndex{};
}

QModelIndex measurement_treemodel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
    {
        return {};
    }
    auto child_item = static_cast<content_presenter *>(child.internalPointer());
    auto parent_item = child_item->parent();
    if (parent_item == _root.get())
    {
        return {};
    }
    return createIndex(parent_item->row(), 0, parent_item);
}

int measurement_treemodel::rowCount(const QModelIndex &parent) const
{
    auto parent_item = parent.isValid() ? static_cast<content_presenter *>(parent.internalPointer()) : _root.get();
    return parent_item ? parent_item->row_count() : 0;
}

int measurement_treemodel::columnCount(const QModelIndex &parent) const
{
    return 8;
}

QVariant data_element(const QModelIndex &index)
{
    if (index.isValid())
    {
        auto ptr = static_cast<content_presenter *>(index.internalPointer());
        return ptr->content(index.column());
    }
    return {};
}

int data_alignment(const QModelIndex &index)
{
    if (index.isValid())
    {
        auto ptr = static_cast<content_presenter *>(index.internalPointer());
        return ptr->alignment(index.column());
    }
    return {};
}

QVariant measurement_treemodel::data(const QModelIndex &index, int role) const
{
    QVariant out;
    switch (role)
    {
    case Qt::ItemDataRole::DisplayRole:
        out = data_element(index);
        break;
    case Qt::ItemDataRole::TextAlignmentRole:
        out = data_alignment(index);
        break;
    }
    return out;
}

QString tree_header(int index)
{
    constexpr char degree[3]{char(0xC2), char(0xB0)};
    switch (index)
    {
    case 0:
        return "Обсерватория";
    case 1:
        return "X,м";
    case 2:
        return "Y,м";
    case 3:
        return "Z,м";
    case 4:
        return "T";
    case 5:
        return "Блеск";
    case 6:
        return QString{"Склон.,"} + degree;
    case 7:
        return QString{"Восх.,"} + degree;
    default:
        throw std::out_of_range{std::to_string(index) + " за пределами диапазона."};
    }
}

QVariant measurement_treemodel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return tree_header(section);
    }
    else
    {
        return {};
    }
}

Qt::ItemFlags measurement_treemodel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index);
}

// таблица и дерево

tle_tableview::tle_tableview(const computational_model *cmodel, QWidget *parent) : QTableView(parent)
{
    verticalHeader()->setVisible(true);
    setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    setModel(new tle_tablemodel(cmodel, this));
    connect(
        model(), &QAbstractTableModel::dataChanged, this,
        [this](const QModelIndex &, const QModelIndex &, const QVector<int> &)
        { resize_to_contents(); });
}

void tle_tableview::resize_to_contents()
{
    resizeColumnsToContents();
    resizeRowsToContents();
}

measurement_treeview::measurement_treeview(const computational_model *cmodel, QWidget *parent)
{
    header()->setStretchLastSection(false);
    setModel(new measurement_treemodel(cmodel, this));
    connect(
        model(), &QAbstractItemModel::dataChanged, this,
        [this](const QModelIndex &, const QModelIndex &, const QVector<int> &)
        { resize_to_contents(); });
    connect(this, &QTreeView::expanded, this, [this](auto)
            { resize_to_contents(); });
}

void measurement_treeview::resize_to_contents()
{
    for (int i{}; i < model()->columnCount(); ++i)
    {
        resizeColumnToContents(i);
    }
}