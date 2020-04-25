#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <future>
#include <mutex>
#include <functional>
#include <vector>
#include <algorithm>

class Result {
public:
    Result() {}
    void plus(int in) {
        std::lock_guard <std::mutex> lock(m);
        result += in;
    }
    int getResult() {
        std::lock_guard <std::mutex> lock(m);
        return result;
    }
private:
    std::mutex m;
    int result = 0;
};

void part1(int block_size, Result& result) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 2.0);
    int in = 0;
    for (int n = 0; n < block_size; ++n) {
        double x = dis(gen);
        double y = dis(gen);
        if (((x - 1) * (x - 1) + (y - 1) * (y - 1)) < 1.0) {
            ++in;
        }
    }
    result.plus(in);
}

int main() {
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0)
        num_threads = 2;
    int all = 100000;
    int block_size = all / num_threads;
    std::vector < std::thread > threads(num_threads - 1);
    Result res;
    for (int i = 0; i < (num_threads - 1); ++i) {
        threads.push_back(std::thread(part1, block_size, std::ref(res)));
    }
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    part1(all - block_size * (num_threads - 1), res);
    std::cout << 4.0 * res.getResult() / all;
    system("pause");
    return 0;
}
