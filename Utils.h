//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_UTILS_H
#define SSGS_UTILS_H

#include <vector>
#include <cstdlib>

using namespace std;

namespace Utils {
    vector<string> readLines(string filename);
    int extractIntFromStr(string s, string rx);
    vector<int> extractIntsFromLine(string line);

    template<class T>
    void batchResize(int size, initializer_list<vector<T> *> vecs) {
        for(auto v : vecs) v->resize(size);
    }

	inline int max(int a, int b) { return a > b ? a : b; }
	inline int max(int a, int b, int c) { return max(max(a, b), c); }
	inline int min(int a, int b) { return a < b ? a : b; }
	inline int min(int a, int b, int c) { return min(min(a, b), c); }

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

	int pickWithDistribution(vector<float> probs);

    void spit(const string s, const string filename);
}

#endif //SSGS_UTILS_H
