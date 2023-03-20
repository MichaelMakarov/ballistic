#include <computation.hpp>
#include <iostream>
#include <config.hpp>
#include <timer.hpp>

int main(int argc, char **argv)
{
    watchtimer timer;
    figure_provider provider{argc, argv};
    try
    {
        configurer conf{"config.txt"};
        std::cout << "Configuration loaded. " << timer << std::endl;
        compute_motion(conf.get_motion_measurements(), conf.get_computationlog_filepath(), provider);
        std::cout << "Computations finished. " << timer << std::endl;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}