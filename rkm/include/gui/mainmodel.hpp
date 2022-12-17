#pragma once
#include <qobject.h>

struct orbit_data;
struct observation_seance;

class computational_model : public QObject {
    Q_OBJECT

    /**
     * @brief Класс для вычислений
     */
    class ballistic_computer* _computer;
    /**
     * @brief Контекст выполнения вычисления
     */
    class computational_context* _context;
    /**
     * @brief Длина мерного интервала в сутках
     */
    double _interval{ 1 };
    /**
     * @brief Индекс опорного ТЛЕ
     */
    int _index{ 0 };
public:
    computational_model(QObject* parent = nullptr);
    ~computational_model();
    /**
     * @brief Чтение данных о ГПЗ из файла.
     * 
     * @param filename 
     */
    void read_gpt() const;
    /**
     * @brief Чтение массива ТЛЕ из файла. 
     * 
     * @param filename 
     */
    void read_tle() const;
    /**
     * @brief Чтение данных измерений.
     * 
     * @param mfilename файл с сеансами измерений
     * @param ofilename файл с обсерваториями
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
    void compute(const std::string& filename) const;
    /**
     * @brief Возвращает ТЛЕ по инлексу.
     * 
     * @param index 
     * @return const orbit_data& 
     */
    const orbit_data& tle_by_index(int index) const;
    /**
     * @brief Возвращает сеанс по индексу.
     * 
     * @param index 
     * @return const observation_seance& 
     */
    const observation_seance& seance_by_index(int index) const;
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
    void computation_performed(const class computational_output*) const;
};




