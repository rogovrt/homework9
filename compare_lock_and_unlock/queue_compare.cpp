#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include <algorithm>
#include <random>
#include <atomic>
#include <vector>
#include <queue>
#include <stdexcept>
#include <boost/lockfree/queue.hpp>

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
class Threadsafe_Queue
{
public:

	Threadsafe_Queue() = default;

	Threadsafe_Queue(const Threadsafe_Queue& other)
	{
		std::lock_guard < std::mutex > lock(other.m_mutex);
		m_queue = other.m_queue;
	}

	Threadsafe_Queue& operator=(const Threadsafe_Queue& other)
	{
		std::lock(m_mutex, other.m_mutex);
		m_queue = other.m_queue;
	}

public:

	void push(T value)
	{
		std::lock_guard < std::mutex > lock(m_mutex);
		m_queue.push(value);
		m_condition_variable.notify_one();
	}

	void pop(T& value)
	{
		std::unique_lock < std::mutex > lock(m_mutex);

		m_condition_variable.wait(lock, [this] {return !m_queue.empty(); });
		value = m_queue.front();
		m_queue.pop();
	}

	std::shared_ptr < T > wait_and_pop()
	{
		std::unique_lock < std::mutex > lock(m_mutex);

		m_condition_variable.wait(lock, [this] {return !m_queue.empty(); });
		auto result = std::make_shared < T >(m_queue.front());
		m_queue.pop();

		return result;
	}

	bool try_pop(T& value)
	{
		std::lock_guard < std::mutex > lock(m_mutex);

		if (m_queue.empty())
		{
			return false;
		}

		value = m_queue.front();
		m_queue.pop();

		return true;
	}

	std::shared_ptr < T > try_pop()
	{
		std::lock_guard < std::mutex > lock(m_mutex);

		if (m_queue.empty())
		{
			return std::shared_ptr < T >();
		}

		auto result = std::make_shared < T >(m_queue.front());
		m_queue.pop();

		return result;
	}

	bool empty() const
	{
		std::lock_guard < std::mutex > lock(m_mutex);
		return m_queue.empty();
	}

private:

	std::queue < T >		m_queue;
	std::condition_variable m_condition_variable;

private:

	mutable std::mutex m_mutex;
};

std::atomic < bool > flag(false);

//using queue_t = Threadsafe_Queue < int >;
using queue_t = boost::lockfree::queue <int, boost::lockfree::capacity<50>>;
queue_t q;

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
		q.push(uid(generator));
	}
}

void pop(const std::size_t size)
{
	while (!flag.load())
	{
		std::this_thread::yield();
	}
	int j;
	for (std::size_t i = 0; i < size; ++i)
	{
		q.pop(j);
	}
}

int main() {
	for (size_t m = 100; m <= 1000000; m *= 10) {
		std::size_t n = 3; //number of threads
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