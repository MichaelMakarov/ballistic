#include <mainview.hpp>
#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    application_window wnd;
    wnd.show();
    return app.exec();
}