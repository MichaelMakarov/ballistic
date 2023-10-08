#include <graphic_drawer.hpp>
#include <graphic_window.hpp>

#include <qapplication.h>

#include <memory>

namespace
{
    std::unique_ptr<QApplication> app;
}

void graphic_drawer::init(int argc, char *argv[])
{
    app = std::make_unique<QApplication>(argc, argv);
}

void graphic_drawer::show()
{
    app->exec();
}

void graphic_drawer::draw(graphic_info const *infos, std::size_t count)
{
    if (count > 0)
    {
        auto wnd = new graphic_window;
        for (std::size_t i{}; i < count; ++i)
        {
            wnd->add_figure(infos[i].row, infos[i].column, infos[i].figure);
        }
        wnd->showMaximized();
    }
}

void graphic_drawer::draw(figure_info const &info)
{
    auto wnd = new graphic_window;
    wnd->add_figure(0, 0, info);
    wnd->showMaximized();
}
