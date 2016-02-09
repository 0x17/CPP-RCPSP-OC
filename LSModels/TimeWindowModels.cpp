
#include "TimeWindowModels.h"

lsdouble ListBetaModel::SerialSGSBetaFunction::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs), beta(p.numJobs);
	if (context.count() < 2 * p.numJobs) return numeric_limits<double>::lowest();
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		beta[i] = static_cast<int>(context.getIntValue(p.numJobs + i));
	}
	auto result = p.serialSGSTimeWindowBorders(order, beta, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

void ListBetaModel::addAdditionalData(LSExpression& obj) {
	for (int i = 0; i < p.numJobs; i++) {
		betaVar[i] = model.boolVar();
		obj.addOperand(betaVar[i]);
	}
}

vector<int> ListBetaModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs), betaVarVals(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
		betaVarVals[i] = static_cast<int>(sol.getIntValue(betaVar[i]));
	}
	return p.serialSGSTimeWindowBorders(order, betaVarVals, true).first;
}

lsdouble ListTauModel::SerialSGSTauFunction::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	vector<float> tau(p.numJobs);
	if (context.count() < 2 * p.numJobs) return numeric_limits<double>::lowest();
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		tau[i] = static_cast<float>(context.getDoubleValue(p.numJobs + i));
	}
	auto result = p.serialSGSTimeWindowArbitrary(order, tau, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}



void ListTauModel::addAdditionalData(LSExpression& obj) {
	for (int i = 0; i < p.numJobs; i++) {
		tauVar[i] = model.floatVar(0.0, 1.0);
		obj.addOperand(tauVar[i]);
	}
}

vector<int> ListTauModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	vector<float> tau(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
		tau[i] = static_cast<float>(sol.getDoubleValue(tauVar[i]));
	}
	return p.serialSGSTimeWindowArbitrary(order, tau, true).first;
}

lsdouble ListTauDiscreteModel::SerialSGSIntegerFunction::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	vector<float> tau(p.numJobs);
	if (context.count() < 2 * p.numJobs) return numeric_limits<double>::lowest();
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
		int beta = static_cast<int>(context.getIntValue(p.numJobs + i));
		tau[i] = (float)(static_cast<double>(beta) / static_cast<double>(IV_COUNT - 1));
	}
	auto result = p.serialSGSTimeWindowArbitrary(order, tau, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

void ListTauDiscreteModel::addAdditionalData(LSExpression& obj) {
	for (int i = 0; i < p.numJobs; i++) {
		tauVar[i] = model.intVar(0, IV_COUNT - 1);
		obj.addOperand(tauVar[i]);
	}
}

vector<int> ListTauDiscreteModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	vector<float> tau(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
		tau[i] = (float)(static_cast<double>(sol.getIntValue(tauVar[i])) / static_cast<double>(IV_COUNT - 1));
	}
	return p.serialSGSTimeWindowArbitrary(order, tau, true).first;
}

lsdouble ListAlternativesModel::SerialSGSAlternativesDecoder::call(const LSNativeContext& context) {
	vector<int> order(p.numJobs);
	if (context.count() < p.numJobs) return numeric_limits<double>::lowest();
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(context.getIntValue(i));
	}
	auto result = p.serialSGSWithOvertime(order, true);
	return static_cast<lsdouble>(p.calcProfit(p.makespan(result.first), result.second));
}

vector<int> ListAlternativesModel::parseScheduleFromSolution(LSSolution &sol) {
	vector<int> order(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
	}
	return p.serialSGSWithOvertime(order, true).first;
}

