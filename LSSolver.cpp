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

    EACH_RNG(j, p.numJobs,
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
    EACH_RNG(j, p.numJobs, Sj[j] = model.intVar(0, p.numPeriods-1));
    vector<vector<LSExpression>> zrt = Utils::initMatrix<LSExpression>(p.numRes, p.numPeriods);
    EACH_RNG(r, p.numRes, EACH_RNG(t, p.numPeriods, zrt[r][t] = model.intVar(0, p.zmax[r])))

    // Objective function
    LSExpression objfunc = model.sum();
    LSExpression revenueExpr = model.createExpression(O_Call, revFuncExpr);
    revenueExpr.addOperand(Sj[p.numJobs-1]);
    objfunc += revenueExpr;
    EACH_RNG(r, p.numRes, EACH_RNG(t, p.numPeriods, objfunc += -p.kappa[r] * zrt[r][t]))

    // Precedence restriction
    EACH_RNG(j, p.numJobs,
        LSExpression lastPred = model.max(0);
        EACH_RNG(i, p.numJobs,
            if(p.adjMx[i][j])
                lastPred.addOperand(Sj[i] + p.durations[i]);
        )
        model.constraint(lastPred <= Sj[j]);
    )

    // Capacity restriction
    EACH_RNG(r, p.numRes,
        EACH_RNG(t, p.numPeriods,
            LSExpression cumDemandExpr = model.createExpression(O_Call, cumDemFuncExpr);
            cumDemandExpr.addOperand(r);
            cumDemandExpr.addOperand(t);
            EACH_RNG(j, p.numJobs, cumDemandExpr.addOperand(Sj[j]))
            model.constraint(cumDemandExpr <= p.capacities[r] + zrt[r][t]);
        )
    )

    model.addObjective(objfunc, OD_Maximize);
    model.close();

    ls.createPhase().setTimeLimit(30);
    ls.getParam().setNbThreads(8);
    ls.solve();

    auto sol = ls.getSolution();
    EACH_RNG(j, p.numJobs, sts[j] = static_cast<int>(sol.getIntValue(Sj[j])));

    //auto status = sol.getStatus();
    //auto solvetime = ls.getStatistics().getRunningTime();

    return sts;
}

#define TIME_WINDOW(j, code) for(int t=p.efts[j]; t<=p.lfts[j]; t++) { code; }
vector<int> LSSolver::solveMIPStyle(ProjectWithOvertime &p) {
    vector<int> sts(p.numJobs);

    LocalSolver ls;
    auto model = ls.getModel();

    // Decision variables
    vector<vector<LSExpression>> x, z;
    Utils::resizeMatrix(x, p.numJobs, p.numPeriods);
    Utils::resizeMatrix(z, p.numRes, p.numPeriods);
    EACH_RNG(j, p.numJobs, EACH_RNG(t, p.numPeriods, x[j][t] = model.boolVar()))
    EACH_RNG(r, p.numRes, EACH_RNG(t, p.numPeriods, z[r][t] = model.intVar(0, p.zmax[r])))

    // Objective function
    auto objfunc = model.sum();
    TIME_WINDOW(p.numJobs-1, objfunc += p.revenue[t] * x[p.numJobs-1][t])
    EACH_RNG(r, p.numRes, EACH_RNG(t, p.numPeriods, objfunc += -p.kappa[r] * z[r][t]))

    // Constraints

    // Each job once
    EACH_RNG(j, p.numJobs,
        auto xsum = model.sum();
        TIME_WINDOW(j, xsum += x[j][t])
        model.constraint(xsum == 1.0);
    )

    // Precedence restrictions
    EACH_RNG(j, p.numJobs,
        EACH_RNG(i, p.numJobs,
            if(p.adjMx[i][j]) {
                auto predFt = model.sum();
                TIME_WINDOW(i, predFt += t * x[i][t])
                auto jobSt = model.sum();
                TIME_WINDOW(j, jobSt += t * x[j][t])
                jobSt += -p.durations[j];
                model.constraint(predFt <= jobSt);
            }
        )
    )

    // Capacity restrictions
    EACH_RNG(r, p.numRes,
        EACH_RNG(t, p.numPeriods,
            auto cumulatedDemand = model.sum();
            EACH_RNG(j, p.numJobs,
                auto makespan = model.sum();
                for(int tau = t; tau < Utils::min(p.numPeriods, p.durations[j]); tau++)
                    makespan += x[j][tau];
                p.demands[j][r] * makespan
            )
            auto totalCapacity = model.sum(p.capacities[r], z[r][t]);
            model.constraint(cumulatedDemand <= totalCapacity);
        )
    )

    model.addObjective(objfunc, OD_Maximize);
    model.close();

    ls.createPhase().setTimeLimit(2);
    ls.getParam().setNbThreads(8);
    ls.solve();

    auto sol = ls.getSolution();
    EACH_RNG(j, p.numJobs,
        EACH_RNG(t, p.numPeriods,
            if(sol.getValue(x[j][t]) == 1) {
                sts[j] = t - p.durations[j];
                break;
            }
        )
    )

    auto status = sol.getStatus();
    if(status != LSSolutionStatus::SS_Feasible) {
        throw runtime_error("No feasible solution found!");
    }

    auto solvetime = ls.getStatistics().getRunningTime();

    return sts;
}
