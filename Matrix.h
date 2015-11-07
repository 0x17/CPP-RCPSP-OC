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
		data = mx.data;
		m = mx.m;
		n = mx.n;
		return *this;
    }

    void resize(int _m, int _n) {
        data.resize(_m*_n);
        m = _m;
        n = _n;
    }

    std::vector<T> row(int i) const {
        std::vector<T> r(n);
        for(int j=0; j<n; j++)
            r[j] = data[i*n+j];
        return r;
    }
};

template<class T>
class Matrix2 {
    int m, n;
    std::vector<std::vector<T>> data;
public:
    Matrix2(const Matrix2& mx) : m(mx.m), n(mx.n), data(mx.data) {}
    Matrix2() : m(0), n(0) {}
    Matrix2(int _m, int _n) : m(_m), n(_n), data(m) {
        for(int i=0; i<m; i++) {
            data[i].resize(n);
        }
    }
    ~Matrix2() {}

    inline T operator()(int i, int j) const { return data[i][j]; }
    inline T &operator()(int i, int j) { return data[i][j]; }

    Matrix2 &operator=(const Matrix2 &mx) {
        Matrix2 m(mx);
        return m;
    }

    void resize(int _m, int _n) {
        //data.resize(_m*_n);

        m = _m;
        n = _n;

        data.resize(_m);
        for(int i=0; i<_m; i++)
            data[i].resize(_n);
    }
};


#endif //CPP_RCPSP_OC_MATRIX_H
