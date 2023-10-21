#pragma once
#include <qwidget.h>

class filepath_view : public QWidget {
  public:
    filepath_view(const QString &filter, const QString &title, QWidget *const parent = nullptr);

    QString get_path() const;

    void set_path(const QString &path);

  private:
    class QLineEdit *_txtbox;
};