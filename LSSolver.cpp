//
// Created by André Schnabel on 24.10.15.
//

#include <localsolver.h>
#include "LSSolver.h"

using namespace localsolver;

class SchedulingNativeFunction : public LSNativeFunction {
protected:
    ProjectWithOvertime &p;
public:
	explicit SchedulingNativeFunction(ProjectWithOvertime &_p) : p(_p) {}
    ~SchedulingNativeFunction() {}
};

class RevenueFunction : public SchedulingNativeFunction {
public:
	explicit RevenueFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) {}
    virtual lsdouble call(const LSNativeContext &context) override;
};

class CumulatedDemandFunction : public SchedulingNativeFunction {
public:
	explicit CumulatedDemandFunction(ProjectWithOvertime &_p) : SchedulingNativeFunction(_p) { }
    virtual lsdouble call(const LSNativeContext &context) override;
};

lsdouble RevenueFunction::call(const LSNativeContext &context) {
    return p.revenue[context.getIntValue(0)];
}

lsdouble CumulatedDemandFunction::call(const LSNativeContext &context) {
    int demand = 0;

    lsint r = context.getIntValue(0);
    lsint t = context.getIntValue(1);

    EACH_COMMON(j, p.numJobs,
        lsint Sj = context.getIntValue(j+2);
        if(Sj < t && t <= Sj + p.durations[j])
            demand += p.demands[j][r];
    )

    return demand;
}

vector<int> LSSolver::solve(ProjectWithOvertime &p) {
    vector<int> sts(p.numJobs);

    LocalSolver ls;
    auto model = ls.getModel();

    RevenueFunction revFunc(p);
    LSExpression revFuncExpr = model.createNativeFunction(&revFunc);

    CumulatedDemandFunction cumDemFunc(p);
    LSExpression cumDemFuncExpr = model.createNativeFunction(&cumDemFunc);

    // Decision variables
    vector<LSExpression> Sj(p.numJobs);
    EACH_COMMON(j, p.numJobs, Sj[j] = model.intVar(0, p.numPeriods-1));
    vector<vector<LSExpression>> zrt = Utils::initMatrix<LSExpression>(p.numRes, p.numPeriods);
    EACH_COMMON(r, p.numRes, EACH_COMMON(t, p.numPeriods, zrt[r][t] = model.intVar(0, p.zmax[r])))

    // Objective function
    LSExpression objfunc = model.sum();
    LSExpression revenueExpr = model.createExpression(O_Call, revFuncExpr);
    revenueExpr.addOperand(Sj[p.numJobs-1]);
    objfunc += revenueExpr;
    EACH_COMMON(r, p.numRes, EACH_COMMON(t, p.numPeriods, objfunc += -p.kappa[r] * zrt[r][t]))

    // Precedence restriction
    EACH_COMMON(j, p.numJobs,
        LSExpression lastPred = model.max(0);
        EACH_COMMON(i, p.numJobs,
            if(p.adjMx[i][j])
                lastPred.addOperand(Sj[i] + p.durations[i]);
        )
        model.constraint(lastPred <= Sj[j]);
    )

    // Capacity restriction
    EACH_COMMON(r, p.numRes,
        EACH_COMMON(t, p.numPeriods,
            LSExpression cumDemandExpr = model.createExpression(O_Call, cumDemFuncExpr);
            cumDemandExpr.addOperand(r);
            cumDemandExpr.addOperand(t);
            EACH_COMMON(j, p.numJobs, cumDemandExpr.addOperand(Sj[j]))
            model.constraint(cumDemandExpr <= p.capacities[r] + zrt[r][t]);
        )
    )

    model.addObjective(objfunc, OD_Maximize);
    model.close();

    ls.createPhase().setTimeLimit(2);
    ls.getParam().setNbThreads(8);
    ls.solve();

    auto sol = ls.getSolution();
    EACH_COMMON(j, p.numJobs, sts[j] = static_cast<int>(sol.getIntValue(Sj[j])));

    //auto status = sol.getStatus();
    //auto solvetime = ls.getStatistics().getRunningTime();

    return sts;
}
