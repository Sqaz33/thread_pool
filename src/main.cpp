#include <iostream>
#include <functional>
#include <future>
#include <array>
#include <initializer_list>
#include <algorithm>
#include <vector>
#include <chrono>

#include "../include/thread_pool.hpp"

// namespace {

struct matrix {
    matrix() = default;

    matrix(size_t Rows, size_t Cols) : 
        m(Rows, std::vector<size_t>(Cols)) 
        , Rows(Rows) 
        , Cols(Cols)
    {}

    matrix(matrix&& other) noexcept : 
        m(std::move(other.m))
        , Rows(other.Rows) 
        , Cols(other.Cols)
    {}

    matrix(const matrix& other) : 
        m(other.m)
        , Rows(other.Rows) 
        , Cols(other.Cols)
    {} 
    

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


// } // namespace 



int main() {
    thread_pool::ThreadPool<void> threadPool;

    // auto id = threadPool.pushTask(sum, 1, 2);
    // int res = 0;
    // threadPool.waitNPopResult(id, res);
    matrix m(100, 100);
    for (size_t i = 0; i < 100; ++i) {
        for (size_t j = 0; j < 100; ++j) {
            m.m[i][j] = i;
        }
    }
    

    std::vector<matrix> m1(100, m);
    auto m2 = m1;
    std::vector<matrix> res(100, m);

    std::vector<size_t> ids;

    auto start = std::chrono::high_resolution_clock::now();


    for (size_t i = 0; i < 100; ++i) {
        ids.push_back(threadPool.pushTask(multiplyMatrix, m1[i], m2[i], res[i]));
    }

    for (auto id : ids) {
        threadPool.waitTask(id);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;

}