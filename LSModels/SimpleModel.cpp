
#include "SimpleModel.h"
#include "../Project.h"
#include <localsolver.h>

using namespace std;
using namespace localsolver;

class ListDecoder : public LSNativeFunction {
	const Project &p;
public:
	ListDecoder(const Project &_p) : p(_p) {}

	lsdouble call(const LSNativeContext & context) override {
		vector<int> order(p.numJobs);

		if (context.count() < p.numJobs) return std::numeric_limits<int>::max();

		for (int i = 0; i < p.numJobs; i++) {
			order[i] = static_cast<int>(context.getIntValue(i));
			if (order[i] == -1) {
				return std::numeric_limits<int>::max();
			}
		}

		vector<int> zeroOc(p.numRes, 0);
		return p.makespan(p.serialSGS(order, zeroOc, true));
	}
};

vector<int> LSSolver::solveRCPSP(const Project &p) {
	LocalSolver ls;
	auto model = ls.getModel();

	ListDecoder sgsFunc(p);
	const LSExpression sgsFuncExpr = model.createNativeFunction(&sgsFunc);
	LSExpression obj = model.createExpression(O_Call, sgsFuncExpr);

	LSExpression activityList = model.listVar(p.numJobs);
	vector<LSExpression> listElems(p.numJobs);
	model.constraint(model.count(activityList) == p.numJobs);

	for (int i = 0; i < p.numJobs; i++) {
		listElems[i] = model.at(activityList, i);
		obj.addOperand(listElems[i]);
	}

	model.addObjective(obj, OD_Minimize);
	model.close();

	auto coll = activityList.getCollectionValue();
	for (int i = 0; i < p.numJobs; i++)
		coll.add(p.topOrder[i]);

	ls.createPhase().setTimeLimit(30);
	auto param = ls.getParam();
	param.setNbThreads(1);
	param.setVerbosity(2);
	ls.solve();

	auto sol = ls.getSolution();

	vector<int> order(p.numJobs);
	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));	

	vector<int> zeroOc(p.numRes, 0);
	return p.serialSGS(order, zeroOc, true).sts;
}