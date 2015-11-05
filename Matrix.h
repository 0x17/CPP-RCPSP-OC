//
// Created by André Schnabel on 05.11.15.
//

#ifndef CPP_RCPSP_OC_MATRIX_H
#define CPP_RCPSP_OC_MATRIX_H

template<class T>
class Matrix {
    int m, n;
    T *data;
public:
    Matrix(const Matrix& mx) : m(mx.m), n(mx.n), data(new T[mx.m*mx.n]) {}
    Matrix() : m(0), n(0) {}
    Matrix(int _m, int _n) : m(_m), n(_n), data(new T[_m*_n]) {}
    ~Matrix() { delete [] data; }

    inline T operator()(int i, int j) const { return data[i*n+j]; }
    inline T &operator()(int i, int j) { return data[i*n+j]; }

    Matrix &operator=(const Matrix &mx) {
        Matrix m(mx);
        return m;
    }

    void resize(int _m, int _n) {
        delete [] data;
        data = new T[_m*_n];
        m = _m;
        n = _n;
    }
};


#endif //CPP_RCPSP_OC_MATRIX_H
