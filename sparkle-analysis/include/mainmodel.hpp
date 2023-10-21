#pragma once

#include <residuals_provider.hpp>
#include <table_data_provider.hpp>
#include <tree_data_provider.hpp>

#include <memory>

class computational_model {
    std::unique_ptr<class ballistic_computer> _computer;
    std::shared_ptr<class table_data_provider_impl> _table_data_provider;
    std::shared_ptr<class tree_data_provider_impl> _tree_data_provider;
    std::shared_ptr<class optimization_logger> _logger;
    double _interval{1};
    std::size_t _index{0};

  public:
    computational_model();
    ~computational_model();

    /**
     * @brief Чтение данных о ГПЗ из файла.
     */
    void read_gpt(std::string const &filepath);

    /**
     * @brief Чтение массива ТЛЕ из файла.
     */
    void read_tle(std::string const &filepath);

    /**
     * @brief Чтение данных измерений.
     */
    void read_measurements(std::string const &obs_filepath, std::string const &meas_filepath);

    /**
     * @brief Задание опорного ТЛЕ по номеру.
     * @param number
     */
    void select_tle(std::size_t number);

    /**
     * @brief Задание мерного интервала в сутках
     * @param interval
     */
    void select_interval(double interval);

    /**
     * @brief Запуск вычислений
     */
    void compute(const std::string &filename);

    std::size_t get_tle_count() const;

    optimization_logger const *get_logger() const;

    std::shared_ptr<table_data_provider> get_table_data_provider() const;

    std::shared_ptr<tree_data_provider> get_tree_data_provider() const;

    std::shared_ptr<residuals_provider> get_residuals_provider() const;
};
