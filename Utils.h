//
// Created by André Schnabel on 23.10.15.
//

#pragma once

#include <cstdlib>
#include <vector>
#include <list>
#include <fstream>
#include <chrono>

#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "Stopwatch.h"

#define LOG_I(msg) Utils::Logger::getInstance()->log(Utils::Logger::LogLevel::INFO, msg)
#define LOG_W(msg) Utils::Logger::getInstance()->log(Utils::Logger::LogLevel::WARNING, msg)
#define LOG_E(msg) Utils::Logger::getInstance()->log(Utils::Logger::LogLevel::ERROR, msg)

namespace Utils {
	std::string slurp(std::string filename);
	std::vector<std::string> readLines(std::string filename);
    int extractIntFromStr(std::string s, std::string rx);
	std::vector<int> extractIntsFromLine(std::string line);

    template<class T>
    void batchResize(int size, std::initializer_list<std::vector<T> *> vecs) {
        for(auto v : vecs) v->resize(size);
    }

	inline int max(int a, int b) { return a > b ? a : b; }
	inline int max(int a, int b, int c) { return max(max(a, b), c); }
	inline int min(int a, int b) { return a < b ? a : b; }
	inline int min(int a, int b, int c) { return min(min(a, b), c); }

	void serializeSchedule(std::vector<int> & sts, const std::string filename);
	void serializeProfit(float profit, const std::string filename);

    inline bool randBool() {
        return rand() % 2 == 0;
    }

    inline int randRangeIncl(int lb, int ub) {
        return lb + rand() % (ub-lb+1);
    }

    inline float randUnitFloat() {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    int pickWithDistribution(std::vector<float> &probs, float q = randUnitFloat());

    template<class T>
    bool rangeInclContains(const std::vector<T> &elems, int lb, int ub, int e) {
        for(int i=lb; i<=ub; i++)
            if(elems[i] == e) return true;
        return false;
    }

	template<class T>
	int indexOfNthEqualTo(int nth, T val, const std::vector<T> & coll) {
		int xth = 0;
		for (int k = 0; k < coll.size(); k++)
			if (coll[k] == val) {
				if (xth == nth) return k;
				xth++;
			}
		throw std::runtime_error("No nth found!");
	}

	template<class T>
	int indexOfFirstEqualTo(T val, const std::vector<T> & coll) {
		return indexOfNthEqualTo(0, val, coll);
    }

    void spit(const std::string s, const std::string filename);
    void spitAppend(const std::string s, const std::string filename);

    template<class T>
    void swap(std::vector<T> &v, int i1, int i2) {
        T tmp = v[i1];
        v[i1] = v[i2];
        v[i2] = tmp;
    }

    template<class Func>
    float maxInRangeIncl(int lb, int ub, Func transform) {
        float r = std::numeric_limits<float>::lowest();
        for(int i = lb; i<=ub; i++) {
            float v = transform(i);
            if(v > r) r = v;
        }
        return r;
    }

	template<class Func, class A, class B>
	std::vector<B> mapVec(Func f, const std::vector<A> &elems) {
		std::vector<B> res(elems.size());
		for (int i = 0; i < elems.size(); i++)
			res[i] = f(elems[i]);
		return res;
    }

	template<class Func, class A, class B>
	std::list<B> mapLst(Func f, std::list<A> &elems) {
		std::list<B> res;
		for(auto elem : elems)
			res.push_back(elem);
		return res;
	}

	template<class A, class Func>
	std::vector<A> constructVector(int size, Func f) {
		std::vector<A> v(size);
		for(int i=0; i<size; i++) {
			v[i] = f(i);
		}
		return v;
    }

	template<class A, class Func>
	std::list<A> constructList(int size, Func f) {
		std::list<A> l;
		for(int i=0; i<size; i++) {
			l.push_back(f(i));
		}
		return l;
    }

	std::list<std::string> filenamesInDir(const std::string &dir);
	std::list<std::string> filenamesInDirWithExt(const std::string& dir, const std::string& ext);

    class Tracer {
		std::ofstream f;
		std::chrono::time_point<std::chrono::system_clock> lupdate;
		double last_slvtime;
		Stopwatch sw;
    public:
        Tracer(const std::string filePrefix = "SolverTrace");
        ~Tracer();
        void trace(double slvtime, float bks_objval, bool trunc_secs = false);
	    void intervalTrace(float bks_objval);
    };

	inline bool int2bool(int i) {
		return i != 0;
	}

	inline int bool2int(bool b) {
		return b ? 1 : 0;
	}

	std::string formattedNow();

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
		std::string logName;
		std::ofstream f;
		LogMode mode;
		static Logger *instance;

	public:
		Logger(const std::string& _logName, LogMode _mode);
		void log(LogLevel level, const std::string& message);

		static Logger* getInstance();
	};

	struct BasicSolverParameters {
		double timeLimit;
		int iterLimit;
		bool traceobj;
		std::string outPath;
		int threadCount;

		BasicSolverParameters(double time_limit, int iter_limit, bool traceobj, const std::string& out_path, int thread_count);
	};

	void partitionDirectory(const std::string& dirPath, int numPartitions, const std::string& infix = "_");

	std::vector<int> deserializeSchedule(int njobs, const std::string &filename);

	std::vector<std::string> splitLines(const std::string& s);

	template<class T, class Pred>
	int indexOf(std::vector<T> elems, Pred p) {
		for(int i=0; i<elems.size(); i++) {
			if (p(elems[i]))
				return i;
		}
		return -1;
	}
}
