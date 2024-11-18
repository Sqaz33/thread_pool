#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include "../include/thread_pool.hpp"

namespace {

struct matrix {
    matrix() = default;

    matrix(size_t Rows, size_t Cols)
        : m(Rows, std::vector<size_t>(Cols)), Rows(Rows), Cols(Cols) {}

    matrix(matrix&& other) noexcept
        : m(std::move(other.m)), Rows(other.Rows), Cols(other.Cols) {}

    matrix(const matrix& other)
        : m(other.m), Rows(other.Rows), Cols(other.Cols) {}

    const size_t Rows = 0;
    const size_t Cols = 0;
    std::vector<std::vector<size_t>> m;
};

void multiplyMatrix(const matrix& a, const matrix& b, matrix& res) {
    if (a.Rows != b.Cols) {
        throw "1234";
    }
    size_t r;
    for (size_t i = 0; i < a.Rows; ++i) {
        for (size_t j = 0; j < b.Cols; ++j) {
            r = 0;
            for (size_t k = 0; k < a.Cols; ++k) {
                r += a.m[i][k] * b.m[k][j];
            }
            res.m[i][j] = r;
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    int mSize = 100;
    int mCount = 1000;
    size_t threadCount = std::thread::hardware_concurrency();
    if (argc > 2) {
        std::string str(argv[1]);
        mSize = std::stoi(str);
        if (argc >= 3) {
            str = argv[2];
            mCount = std::stoi(str);
        }
        if (argc >= 4) {
            str = argv[3];
            threadCount = std::stoi(str);
        }
    }

    thread_pool::ThreadPool<void> threadPool(threadCount);

    matrix m(mSize, mSize);
    for (size_t i = 0; i < mSize; ++i) {
        for (size_t j = 0; j < mSize; ++j) {
            m.m[i][j] = i;
        }
    }

    std::vector<matrix> m1(mCount, m);
    auto m2 = m1;
    std::vector<matrix> res(mCount, m);

    std::vector<size_t> ids;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < mCount; ++i) {
        if (threadCount > 0) {
            ids.push_back(
                threadPool.pushTask(multiplyMatrix, m1[i], m2[i], res[i]));
        } else {
            multiplyMatrix(m1[i], m2[i], res[i]);
        }
    }

    for (auto id : ids) {
        threadPool.waitTask(id);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
}