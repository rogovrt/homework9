#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <future>
#include <algorithm>

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

class stringPart {
public:
	stringPart(std::string s, int num) {
		this->s = s;
		this->start = num;
	}

	stringPart(const stringPart&) = delete;

	std::vector<int> find(std::string req, int pos) {
		std::vector<int> index;
		int i = pos;
		//std::lock_guard <std::mutex> lock(m);
		m.lock();
		for (i = s.find(req, i++); i != std::string::npos; i = s.find(req, i + 1))
			index.push_back(start + i);
		m.unlock();
		return index;
	}
	int str_len() {
		return s.length();
	}
private:
	std::mutex m;
	std::string s;
	int start;
};

void part(std::vector<std::unique_ptr<stringPart>>& parts, std::string req, int i, std::vector<int>& res) {
	std::vector <int> res0 = parts.at(i) -> find(req, 0);
	for (int j = 1; j < req.length(); ++j) {
		std::vector <int> res1 = parts.at(i) -> find(req.substr(0, j), parts.at(i) -> str_len() - req.length() - j);
		std::vector <int> res2 = parts.at(i + 1) -> find(req.substr(j - 1, req.length() - j), 0);
		if (!res1.empty() && !res2.empty())
			if (res2.at(0) == 0)
				res0.push_back(res1.at(0));
	}
	std::mutex m;
	std::lock_guard <std::mutex> lock(m);
	res.insert(res.end(), res0.begin(), res0.end());
}

int main() {
	std::string base = generate_str(4000);
	std::string required;
	std::cin >> required;

	int num_threads = std::thread::hardware_concurrency();
	if (num_threads == 0)
		num_threads = 2;
	int all = required.length();
	int block_size = all / num_threads;
	std::vector < std::thread > threads(num_threads - 1);
	std::vector <std::unique_ptr<stringPart>> parts(num_threads);
	std::vector <int> res;
	for (int i = 0; i < num_threads; ++i) {
		//stringPart p(base.substr(i * block_size, block_size), i * block_size);
		parts.push_back(std::make_unique<stringPart>(base.substr(i * block_size, block_size), i * block_size));
		if (i == num_threads - 1)
			//stringPart p1(base.substr(i * block_size, base.length() - (num_threads - 1) * block_size), i * block_size);
			parts.push_back(std::make_unique<stringPart>(base.substr(i * block_size, base.length() - (num_threads - 1) * block_size), i * block_size));
	}
	try {
		for (int i = 0; i < (num_threads - 1); ++i) {
			std::packaged_task <void(std::vector<std::unique_ptr<stringPart>>&, std::string, int, std::vector<int>&)> task(part);
			threads[i] = std::thread(std::move(task), std::ref(parts), required, i, std::ref(res));
		}
		std::for_each(threads.begin(), threads.end(),
			std::mem_fn(&std::thread::join));
	}
	catch (...) {
		for (unsigned long i = 0; i < (num_threads - 1); ++i)
		{
			if (threads[i].joinable())
				threads[i].join();
		}
		throw;
	}
	for (auto el : res)
		std::cout << el << std::endl;
	return 0;
}