//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_UTILS_H
#define SSGS_UTILS_H

#include <string>
#include <vector>
#include <cstdlib>
#include <list>
#include <boost/format.hpp>
#include <fstream>

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
	void serializeProfit(float profit, const string filename);

    inline bool randBool() {
        return rand() % 2 == 0;
    }

    inline int randRangeIncl(int lb, int ub) {
        return lb + rand() % (ub-lb+1);
    }

    inline float randUnitFloat() {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    int pickWithDistribution(vector<float> &probs, float q = randUnitFloat());

    template<class T>
    bool rangeInclContains(const vector<T> &elems, int lb, int ub, int e) {
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

    void spit(const string s, const string filename);
    void spitAppend(const string s, const string filename);

    template<class T>
    void swap(vector<T> &v, int i1, int i2) {
        T tmp = v[i1];
        v[i1] = v[i2];
        v[i2] = tmp;
    }

    template<class Func>
    float maxInRangeIncl(int lb, int ub, Func transform) {
        float r = numeric_limits<float>::lowest();
        for(int i = lb; i<=ub; i++) {
            float v = transform(i);
            if(v > r) r = v;
        }
        return r;
    }

	template<class Func, class A, class B>
	vector<B> mapVec(Func f, vector<A> &elems) {
		vector<B> res(elems.size());
		for (int i = 0; i < elems.size(); i++)
			res[i] = f(elems[i]);
		return res;
    }

	template<class Func, class A, class B>
	list<B> mapLst(Func f, list<A> &elems) {
		list<B> res;
		for(auto elem : elems)
			res.push_back(elem);
		return res;
	}

	list<string> filenamesInDirWithExt(const string dir, const string ext);

    class Tracer {
        ofstream f;
    public:
        Tracer(const string filePrefix = "SolverTrace");
        ~Tracer();
        void trace(double slvtime, float bks_objval);
    };
}

#endif //SSGS_UTILS_H
