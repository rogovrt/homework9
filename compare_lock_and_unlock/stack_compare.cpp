#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include <algorithm>
#include <random>
#include <atomic>
#include <vector>
#include <stack>
#include <stdexcept>
#include <boost/lockfree/stack.hpp>

class Timer {
public:
	Timer() : m_begin(std::chrono::steady_clock::now()), duration(0), is_work(true) {}

	~Timer() {
		if (is_work)
			this->stop();
		std::cout << duration << " milliseconds" << std::endl;
	}

	int stop() {
		auto end = std::chrono::steady_clock::now();
		duration += std::chrono::duration_cast<std::chrono::milliseconds> (end - m_begin).count();
		is_work = false;
		return duration;
	}

	void go() {
		m_begin = std::chrono::steady_clock::now();
		is_work = true;
	}

private:
	std::chrono::steady_clock::time_point m_begin;
	int duration;
	bool is_work;
};

template < typename T >
class threadsafe_stack
{
private:

	std::stack < T > data;
	mutable std::mutex mutex;

public:

	threadsafe_stack()
	{}

	threadsafe_stack(const threadsafe_stack& other)
	{
		std::lock_guard < std::mutex > lock(other.mutex);
		data = other.data;
	}

	threadsafe_stack& operator=(const threadsafe_stack&) = delete;

	void push(T new_value)
	{
		std::lock_guard < std::mutex > lock(mutex);
		data.push(new_value);
	}

	std::shared_ptr < T > pop()
	{
		std::lock_guard < std::mutex > lock(mutex);
		if (data.empty())
			return NULL;
		const std::shared_ptr < T > result(std::make_shared < T >(data.top()));
		data.pop();
		return result;
	}

	void pop(T& value)
	{
		std::lock_guard < std::mutex > lock(mutex);
		value = data.top();
		data.pop();
	}
};

std::atomic < bool > flag(false);

using stack_t = threadsafe_stack < int >;
//using stack_t = boost::lockfree::stack <int, boost::lockfree::capacity<50>>;
stack_t s;

std::random_device rd;
std::mt19937 generator(rd());
std::uniform_int_distribution <> uid(0, 100);


void push(const std::size_t size)
{
	while (!flag.load())
	{
		std::this_thread::yield();
	}

	for (std::size_t i = 0; i < size; ++i)
	{
		s.push(uid(generator));
	}
}

void pop(const std::size_t size)
{
	while (!flag.load())
	{
		std::this_thread::yield();
	}
	//int j;
	for (std::size_t i = 0; i < size; ++i)
	{
		s.pop();
		//s.pop(j);
	}
}

int main() {
	for (size_t m = 100; m <= 1000000; m *= 10) {
		std::size_t n = 6; //number of threads
		//std::size_t m = 100; //number of elems per thread
		std::vector < std::thread > push_threads(n);
		std::vector < std::thread > pop_threads(n);

		for (std::size_t i = 0; i < n; ++i)
		{
			push_threads[i] = std::thread(push, m);
			pop_threads[i] = std::thread(pop, m);
		}

		{
			Timer t;

			flag.store(true);

			std::for_each(push_threads.begin(), push_threads.end(), std::mem_fn(&std::thread::join));
			std::for_each(pop_threads.begin(), pop_threads.end(), std::mem_fn(&std::thread::join));
		}
	}
	return 0;
}