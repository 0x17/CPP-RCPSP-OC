//
// Created by André Schnabel on 05.11.15.
//

#ifndef CPP_RCPSP_OC_MATRIX_H
#define CPP_RCPSP_OC_MATRIX_H

#include <vector>

template<class T>
class Matrix {
    int m, n;
    std::vector<T> data;
public:
    Matrix(const Matrix& mx) : m(mx.m), n(mx.n), data(mx.data) {}
    Matrix() : m(0), n(0) {}
    Matrix(int _m, int _n) : m(_m), n(_n), data(_m*_n) {}
    ~Matrix() {}

    inline T operator()(int i, int j) const { return data[i*n+j]; }
    inline T &operator()(int i, int j) { return data[i*n+j]; }

    Matrix &operator=(const Matrix &mx) {
        Matrix m(mx);
        return m;
    }

    void resize(int _m, int _n) {
        data.resize(_m*_n);
        m = _m;
        n = _n;
    }
};


#endif //CPP_RCPSP_OC_MATRIX_H
