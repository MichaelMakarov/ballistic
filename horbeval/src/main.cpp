#include <mainview.h>

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    mainview wnd;
    wnd.show();
    return app.exec();
}