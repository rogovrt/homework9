#include <iostream>
#include <thread>
#include <future>
#include <queue>

template < typename T, typename Container, typename Compare>
class thread_safe_priority_queque {
private:
	std::mutex m;
	std::condition_variable c;
	std::priority_queue <T, Container, Compare> data;
public:
	thread_safe_priority_queque() {}

	void push(T val) {
		std::lock_guard <std::mutex> lock(m);
		data.push(std::move(val));
		c.notify_one();
	}

	void pop(T& val) {
		std::unique_lock <std::mutex> lock(m);
		c.wait(lock, [this] {return !data.empty(); });
		val = std::move(data.top());
		data.pop();
	}
};

int main() {
	thread_safe_priority_queque <int, std::vector<int>, std::greater<int>> q;
	q.push(10);
	q.push(5);
	int i, i1;
	std::thread t(&thread_safe_priority_queque <int, std::vector<int>, std::greater<int>> ::pop, &q, std::ref(i));
	std::thread t1(&thread_safe_priority_queque <int, std::vector<int>, std::greater<int>> ::pop, &q, std::ref(i1));
	t.join();
	t1.join();
	std::cout << i << " " << i1 << std::endl;
	return 0;
}