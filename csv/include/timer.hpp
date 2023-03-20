#pragma once
#include <ostream>

/**
 * @brief Таймер
 *
 */
class watchtimer
{
    class watchtimer_impl *_impl;

public:
    watchtimer();
    watchtimer(watchtimer const &);
    watchtimer(watchtimer &&) noexcept;
    ~watchtimer();
    watchtimer &operator=(watchtimer const &);
    watchtimer &operator=(watchtimer &&) noexcept;
    friend std::ostream &operator<<(std::ostream &, watchtimer const &);
};