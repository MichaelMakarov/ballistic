#include <parallel.hpp>
#include <vector>
#include <future>

namespace par
{
    namespace detail
    {

        void execute(invocable *inv)
        {
            while (inv->invoke())
            {
            }
        }

        void parallel_for_impl(invocable &inv, size_t task_count, size_t thread_count)
        {
			std::vector<std::future<void>> futures(std::min(thread_count - 1, task_count - 1));
            for (auto &f : futures)
            {
                f = std::async(std::launch::async, &execute, &inv);
            }
            execute(&inv);
            for (auto &f : futures)
            {
                f.wait();
            }
        }

        void parallel_for_impl(invocable &inv, size_t task_count)
        {
            parallel_for_impl(inv, task_count, std::thread::hardware_concurrency());
        }
    }
}
