#include <optimize.hpp>
#include <string>
#include <queue>
#include <future>
#include <vector>
#include <memory>
#include <atomic>

void throw_if_index_out_of_range(size_t i, size_t supr)
{
	supr -= 1;
	if (i > supr)
	{
		std::string msg = "Инлекс вне диапазона (" + std::to_string(i) + " > " + std::to_string(supr) + ").";
		throw std::invalid_argument(msg);
	}
}

bool is_equal(double oldval, double newval, double eps)
{
	return std::abs(1 - newval / oldval) < eps;
}

bool is_equal_or_greater(double oldval, double newval, double eps)
{
	return oldval < newval || is_equal(oldval, newval, eps);
}

class forward_range
{
	size_t _begin;
	size_t _end;

public:
	forward_range(size_t begin, size_t end) : _begin{begin}, _end{end} {}
	bool empty() const { return _begin >= _end; }
	size_t value() const { return _begin; }
};

class interval_range
{
	std::atomic<size_t> _begin;
	size_t _end;

public:
	interval_range(size_t begin, size_t end)
	{
		std::tie(begin, end) = std::minmax(begin, end);
		_begin = begin;
		_end = end;
	}
	size_t size() const { return _begin < _end ? _end - _begin : 0; }
	forward_range increment() { return forward_range(_begin++, _end); }
};

class parallel_executer
{
	interval_range _range;
	std::unique_ptr<detail::invoker> _invoker;

public:
	parallel_executer(size_t begin, size_t end, detail::invoker *ptr) : _range(begin, end), _invoker{ptr} {}
	void run()
	{
		size_t thread_count = std::thread::hardware_concurrency();
		std::vector<std::future<void>> futures(std::min(thread_count, _range.size() - 1));
		for (auto &f : futures)
			f = std::async(std::launch::async, &parallel_executer::execute, this);
		execute();
		for (auto &f : futures)
			f.wait();
	}

private:
	void execute()
	{
		while (true)
		{
			auto fwd_range = _range.increment();
			if (fwd_range.empty())
				return;
			_invoker->invoke(fwd_range.value());
		}
	}
};

void parallel_for_impl(size_t begin, size_t end, detail::invoker *invoker)
{
	parallel_executer exe(begin, end, invoker);
	exe.run();
}