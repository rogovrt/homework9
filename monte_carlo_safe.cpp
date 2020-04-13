#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <future>
#include <mutex>
#include <functional>

class Result {
public:
    Result() {}
    void plus(int in) {
        std::lock_guard <std::mutex> lock(m);
        result += in;
    }
    int getResult() {
        return result;
    }
private:
    std::mutex m;
    int result = 0;
};

/*void part(int block_size, int& r) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 2.0);
    //std::mutex m;
    int in = 0;
    for (int n = 0; n < block_size; ++n) {
        double x = dis(gen);
        double y = dis(gen);
        if (((x - 1) * (x - 1) + (y - 1) * (y - 1)) < 1.0) {
            std::lock_guard <std::mutex> lock(m);
            r++;
        }
    }
}*/

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
    try {
        for (int i = 0; i < (num_threads - 1); ++i) {
            std::packaged_task <void(int, Result&)> task(part1);
            threads[i] = std::thread(std::move(task), block_size, std::ref(res));
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
    part1(all - block_size * (num_threads - 1), res);
    std::cout << 4.0 * res.getResult() / all;
    return 0;
}