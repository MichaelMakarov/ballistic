#include <iostream>
#include <config.hpp>
#include <timer.hpp>
#include <figure.hpp>

int main(int argc, char **argv)
{
    watchtimer timer;
    figure_provider::initialize(argc, argv);
    try
    {
        application_configurer conf{"config.txt"};
        std::cout << "Configuration loaded. " << timer << std::endl;
        conf.compute();
        std::cout << "Computations finished. " << timer << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}