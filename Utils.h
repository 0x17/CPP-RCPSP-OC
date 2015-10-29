//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_UTILS_H
#define SSGS_UTILS_H

#include <vector>
#include <string>
using namespace std;

namespace Utils {
    vector<string> readLines(string filename);
    int extractIntFromStr(string s, string rx);
    vector<int> extractIntsFromLine(string line);

    template<class T>
    void resizeMatrix(vector<vector<T>> &matrix, int m, int n) {
        matrix.resize(m);
        for(int i = 0; i<m; i++) {
            matrix[i].resize(n);
        }
    }

    template<class T>
    vector<vector<T>> initMatrix(int m, int n) {
        vector<vector<T>> result;
        resizeMatrix(result, m, n);
        return result;
    }

    template<class T>
    void batchResize(int size, initializer_list<vector<T> *> vecs) {
        for(auto v : vecs) v->resize(size);
    }

	inline int max(int a, int b) { return a > b ? a : b; }
	inline int min(int a, int b) { return a < b ? a : b; }

	void serializeSchedule(vector<int> & sts, const string filename);
}

#endif //SSGS_UTILS_H
