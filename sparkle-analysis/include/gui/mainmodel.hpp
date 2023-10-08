#pragma once

#include <logger.hpp>
#include <observation.hpp>

#include <qobject.h>

#include <memory>

class computational_model : public QObject
{
    Q_OBJECT

    /// @brief Класс для вычислений
    std::unique_ptr<class ballistic_computer> _computer;
    /// @brief Логировщик
    std::unique_ptr<optimization_logger> _logger;
    /// @brief Интервал в сутках
    double _interval{1};
    /// @brief Индекс опорного ТЛЕ
    int _index{0};

public:
    computational_model(QObject *parent = nullptr);
    ~computational_model();
    /**
     * @brief Чтение данных о ГПЗ из файла.
     */
    void read_gpt() const;
    /**
     * @brief Чтение массива ТЛЕ из файла.
     */
    void read_tle() const;
    /**
     * @brief Чтение данных измерений.
     */
    void read_measurements() const;
    /**
     * @brief Задание опорного ТЛЕ по номеру.
     *
     * @param number
     */
    void select_tle(int number);
    /**
     * @brief Задание мерного интервала в сутках
     *
     * @param interval
     */
    void select_interval(double interval);
    /**
     * @brief Запуск вычислений
     *
     */
    void compute(const std::string &filename);
    /**
     * @brief Возвращает ТЛЕ по инлексу.
     *
     * @param index
     * @return const orbit_data&
     */
    const orbit_data &tle_by_index(int index) const;
    /**
     * @brief Возвращает сеанс по индексу.
     *
     * @param index
     * @return const observation_seance&
     */
    const observation_seance &seance_by_index(int index) const;
    /**
     * @brief Кол-во ТЛЕ
     *
     * @return int
     */
    int tle_count() const;
    /**
     * @brief Кол-во сеансов
     *
     * @return int
     */
    int seance_count() const;

    optimization_logger const *get_logger() const;

signals:
    /**
     * @brief Сигнал о том, что ТЛЕ загружены
     *
     */
    void tle_data_loaded() const;
    /**
     * @brief Сигнал о том, что данные сеансов измерений загружены
     *
     */
    void measurement_data_loaded() const;
    /**
     * @brief Сигнал о том, что данные ГПЗ загружены
     *
     */
    void gpt_data_loaded() const;
    /**
     * @brief Сигнал о том, что вычисление завершено
     *
     */
    void computation_performed(const class computational_output *) const;
};
