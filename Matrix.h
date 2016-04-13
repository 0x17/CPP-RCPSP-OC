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
    template<class Func>
    Matrix(int _m , int _n, Func code) : m(_m), n(_n), data(_m*_n) {
        for(int i=0; i<m; i++)
            for(int j=0; j<n; j++)
                data[i*n+j] = code(i,j);
    }

    Matrix(const Matrix& mx) : m(mx.m), n(mx.n), data(mx.data) {}

    Matrix(const std::vector<std::vector<T>> &rows)
    : m(static_cast<int>(rows.size())), n(static_cast<int>(rows[0].size())), data(m*n) {
        for(int i=0; i<m; i++)
            for(int j=0; j<n; j++)
                data[i*n+j] = rows[i][j];
    }

    Matrix() : m(0), n(0) {}

    Matrix(int _m, int _n) : m(_m), n(_n), data(_m*_n) {}

    ~Matrix() {}

    int getM() const { return m; }
    int getN() const { return n; }

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

	std::vector<T> column(int j) const {
		std::vector<T> c(m);
		for (int i = 0; i<m; i++)
			c[i] = data[i*n + j];
		return c;
    }
};


#endif //CPP_RCPSP_OC_MATRIX_H
