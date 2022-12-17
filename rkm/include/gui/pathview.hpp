#pragma once
#include <qwidget.h>

class filepath_view : public QWidget {
    Q_OBJECT
public:
    filepath_view(const QString& filter, const QString& title, QWidget* const parent = nullptr);
    QString get_path() const;
    void set_path(const QString& path);
signals:
    void path_changed(const QString&);
private:
    class QLineEdit* _txtbox;
};