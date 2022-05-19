#pragma once

#include <qwidget.h>

class pathview : public QWidget {
    Q_OBJECT
public:
    pathview(const QString& filter, const QString& title, QWidget* const parent = nullptr);
    QString get_path() const;
    void set_path(const QString& path);
signals:
    void path_changed(const QString&);
private:
    class QLineEdit* _txtbox;
};