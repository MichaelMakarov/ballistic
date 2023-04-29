#include <iostream>
#include <config.hpp>
#include <timer.hpp>
#include <figure.hpp>

void compute_motion(std::vector<motion_measurement> const &measurements, std::filesystem::path const &filepath);

int main(int argc, char **argv)
{
    watchtimer timer;
    figure_provider::initialize(argc, argv);
    try
    {
        configurer conf{"config.txt"};
        std::cout << "Configuration loaded. " << timer << std::endl;
        compute_motion(conf.get_motion_measurements(), conf.get_computationlog_filepath());
        std::cout << "Computations finished. " << timer << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}