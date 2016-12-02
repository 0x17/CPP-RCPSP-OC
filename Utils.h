//
// Created by André Schnabel on 23.10.15.
//

#ifndef SSGS_UTILS_H
#define SSGS_UTILS_H

#include <vector>
#include <cstdlib>
#include <list>
#include <boost/format.hpp>
#include <fstream>
#include <chrono>
#include "Stopwatch.h"

using namespace std;

#define LOG_I(msg) Utils::Logger::getInstance()->log(Utils::Logger::LogLevel::INFO, msg)
#define LOG_W(msg) Utils::Logger::getInstance()->log(Utils::Logger::LogLevel::WARNING, msg)
#define LOG_E(msg) Utils::Logger::getInstance()->log(Utils::Logger::LogLevel::ERROR, msg)

namespace Utils {
	string slurp(string filename);
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

	template<class A, class Func>
	vector<A> constructVector(int size, Func f) {
		vector<A> v(size);
		for(int i=0; i<size; i++) {
			v[i] = f(i);
		}
		return v;
    }

	template<class A, class Func>
	list<A> constructList(int size, Func f) {
		list<A> l;
		for(int i=0; i<size; i++) {
			l.push_back(f(i));
		}
		return l;
    }

	list<string> filenamesInDir(const string &dir);
	list<string> filenamesInDirWithExt(const string& dir, const string& ext);

    class Tracer {
        ofstream f;
		std::chrono::time_point<std::chrono::system_clock> lupdate;
		double last_slvtime;
		Stopwatch sw;
    public:
        Tracer(const string filePrefix = "SolverTrace");
        ~Tracer();
        void trace(double slvtime, float bks_objval, bool trunc_secs = false);
	    void intervalTrace(float bks_objval);
    };

	inline bool int2bool(int i) {
		return i != 0;
	}

	string formattedNow();

	class Logger {
	public:
		enum class LogLevel {
			INFO = 0,
			WARNING,
			ERROR
		};

		enum class LogMode {
			QUIET = 0,
			MEDIUM,
			VERBOSE
		};

	private:
		string logName;
		ofstream f;
		LogMode mode;
		static Logger *instance;

	public:
		Logger(const string& _logName, LogMode _mode);
		void log(LogLevel level, const string& message);

		static Logger* getInstance();
	};

	struct BasicSolverParameters {
		double timeLimit;
		int iterLimit;
		bool traceobj;
		string outPath;
		int threadCount;

		BasicSolverParameters(double time_limit, int iter_limit, bool traceobj, const string& out_path, int thread_count);
	};

	void partitionDirectory(const string& dirPath, int numPartitions, const string& infix = "_");
}

#endif //SSGS_UTILS_H
