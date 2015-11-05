//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_UTILS_H
#define SSGS_UTILS_H

#include <vector>
#include <string>
#include <cstdlib>
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

    inline int randRangeIncl(int lb, int ub) {
        return lb + rand() % (ub-lb+1);
    }

    inline float randUnitFloat() {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    template<class T>
    bool rangeContains(const vector<T> &elems, int lb, int ub, int e) {
        for(int i=lb; i<=ub; i++)
            if(elems[i] == e) return true;
        return false;
    }

	template<class T>
	int indexOfNthEqualTo(int nth, T val, vector<T> & coll) {
		int xth = 0;
		for (int k = 0; k < coll.size(); k++)
			if (coll[k] == val) {
				if (xth == nth) return k;
				xth++;
			}
		throw runtime_error("No nth found!");
	}
}

#endif //SSGS_UTILS_H
