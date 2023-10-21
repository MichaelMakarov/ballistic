#include <residual_graphic.hpp>
#include <residuals_iterator.hpp>

#include <maths.hpp>

namespace {
    
    template <typename iterator>
    void add_figure(iterator begin,
                    iterator end,
                    std::string const &title,
                    axis_info const &x_info,
                    axis_info const &y_info,
                    graphic_window *wnd,
                    unsigned row,
                    unsigned column) {
        std::vector<double> x_values;
        std::vector<double> y_values;
        for (std::size_t i{}; begin != end; ++begin) {
            x_values.push_back(i++);
            y_values.push_back(math::rad_to_deg(*begin));
        }
        figure_info info{.title = title,
                         .points = points_info{.x_array = x_values.data(), .y_array = y_values.data(), .count = x_values.size()},
                         .x_axis = x_info,
                         .y_axis = y_info};
        wnd->add_figure(row, column, info);
    }

} // namespace

graphic_window *make_residuals_window(residuals_provider const &provider) {
    axis_info const x_axis{.name = "Невязка"};
    axis_info const incl_axis{.name = "|i|, град"};
    axis_info const asc_axis{.name = "|a|, град"};

    auto wnd = new graphic_window;
    auto residuals = provider.get_first_iteration_residuals();
    add_figure(residuals_iterator<&residual_point::i>{residuals.data()},
               residuals_iterator<&residual_point::i>{residuals.data() + residuals.size()},
               "Невязки по склонению на первой итерации",
               x_axis,
               incl_axis,
               wnd,
               0,
               0);
    add_figure(residuals_iterator<&residual_point::a>{residuals.data()},
               residuals_iterator<&residual_point::a>{residuals.data() + residuals.size()},
               "Невязки по восхождению на первой итерации",
               x_axis,
               asc_axis,
               wnd,
               0,
               1);

    residuals = provider.get_last_iteration_residuals();
    add_figure(residuals_iterator<&residual_point::i>{residuals.data()},
               residuals_iterator<&residual_point::i>{residuals.data() + residuals.size()},
               "Невязки по склонению на последней итерации",
               x_axis,
               incl_axis,
               wnd,
               1,
               0);
    add_figure(residuals_iterator<&residual_point::a>{residuals.data()},
               residuals_iterator<&residual_point::a>{residuals.data() + residuals.size()},
               "Невязки по восхождению на последней итерации",
               x_axis,
               asc_axis,
               wnd,
               1,
               1);
    return wnd;
}