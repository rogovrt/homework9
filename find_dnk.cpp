#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <future>
#include <algorithm>

std::mutex m;

class Thread_Guard
{
public:

	explicit Thread_Guard(std::thread& thread) noexcept :
		m_thread(thread)
	{}

	Thread_Guard(const Thread_Guard&) = delete;

	Thread_Guard& operator=(const Thread_Guard&) = delete;

	~Thread_Guard() noexcept
	{
		try
		{
			if (m_thread.joinable())
			{
				m_thread.join();
			}
		}
		catch (...)
		{
			// std::abort();
		}
	}

private:

	std::thread& m_thread;
};


std::string generate_str(int l) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 3);
	std::string s;
	for (int i = 0; i < l; ++i) {
		int n = dis(gen);
		switch (n) {
		case 0:
			s.push_back('A');
		case 1:
			s.push_back('G');
		case 2:
			s.push_back('T');
		case 3:
			s.push_back('C');
		}
	}
	return s;
}

void find_str(int first, int last, std::vector <int>& res, std::string& req, std::string& base) {
	for (int i = base.find(req, first); (i < last) && (i != std::string::npos); i = base.find(req, i + 1)) {
		std::lock_guard <std::mutex> lock(m);
			res.push_back(i);
	}
}



int main() {
	std::string base = generate_str(4000);
	std::string required;
	std::cin >> required;

	int num_threads = std::thread::hardware_concurrency();
	if (num_threads == 0)
		num_threads = 2;
	int all = base.length();
	int block_size = all / num_threads;
	std::vector < std::thread > threads(num_threads - 1);
	std::vector < int > res;
	for (int i = 0; i < num_threads - 1; ++i) {
		threads[i] = std::thread(find_str, i * block_size, (i + 1) * block_size, std::ref(res), std::ref(required), std::ref(base));
		Thread_Guard guard(threads[i]);
	}

	int j = 0;
	std::vector < int > res_compare;
	for (j = base.find(required, j++); j != std::string::npos; j = base.find(required, j + 1))
		res_compare.push_back(j);

	if (std::equal(res.begin(), res.end(), res_compare.begin()))
		std::cout << "Programm works correct\n";
	return 0;
}