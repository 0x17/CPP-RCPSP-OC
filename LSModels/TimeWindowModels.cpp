#ifndef DISABLE_LOCALSOLVER

#include "TimeWindowModels.h"

using namespace std;
using namespace localsolver;

ProjectWithOvertime::BorderSchedulingOptions ListBetaModel::options;

void ListBetaModel::setVariant(int variant) {
	options.setFromIndex(variant);
}

int ListBetaModel::SerialSGSBetaFunction::varCount() {
	return 2 * p.numJobs;
}

SGSResult ListBetaModel::SerialSGSBetaFunction::decode(vector<int>& order, const LSExternalArgumentValues& context) {
	vector<int> beta(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		beta[i] = static_cast<int>(context.getIntValue(p.numJobs + i));
	}
	return p.serialSGSTimeWindowBordersWithForwardBackwardImprovement(order, beta, options, true);
}

void ListBetaModel::addAdditionalData(LSModel &model, LSExpression& obj) {
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
	return p.serialSGSTimeWindowBordersWithForwardBackwardImprovement(order, betaVarVals, options, true).sts;
}

//==============================================================================================================

int ListTauModel::SerialSGSTauFunction::varCount() {
	return 2 * p.numJobs;
}

SGSResult ListTauModel::SerialSGSTauFunction::decode(vector<int>& order, const LSExternalArgumentValues& context) {
	vector<float> tau(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		tau[i] = static_cast<float>(context.getDoubleValue(p.numJobs + i));
	}
	return p.serialSGSTimeWindowArbitraryWithForwardBackwardImprovement(order, tau, true);
}

void ListTauModel::addAdditionalData(LSModel &model, LSExpression& obj) {
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
	return p.serialSGSTimeWindowArbitraryWithForwardBackwardImprovement(order, tau, true).sts;
}

//==============================================================================================================

int ListTauDiscreteModel::SerialSGSIntegerFunction::varCount() {
	return 2 * p.numJobs;
}

SGSResult ListTauDiscreteModel::SerialSGSIntegerFunction::decode(vector<int>& order, const LSExternalArgumentValues& context) {
	vector<float> tau(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		int beta = static_cast<int>(context.getIntValue(p.numJobs + i));
		tau[i] = static_cast<float>(static_cast<double>(beta) / static_cast<double>(IV_COUNT - 1));
	}
	return p.serialSGSTimeWindowArbitraryWithForwardBackwardImprovement(order, tau, true);
}

void ListTauDiscreteModel::addAdditionalData(LSModel &model, LSExpression& obj) {
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
		tau[i] = static_cast<float>(static_cast<double>(sol.getIntValue(tauVar[i])) / static_cast<double>(IV_COUNT - 1));
	}
	return p.serialSGSTimeWindowArbitraryWithForwardBackwardImprovement(order, tau, true).sts;
}

//==============================================================================================================

int ListAlternativesModel::SerialSGSAlternativesDecoder::varCount() {
	return p.numJobs;
}

SGSResult ListAlternativesModel::SerialSGSAlternativesDecoder::decode(vector<int>& order, const LSExternalArgumentValues& context) {
	return p.serialSGSWithOvertimeWithForwardBackwardImprovement(order, true);
}

vector<int> ListAlternativesModel::parseScheduleFromSolution(LSSolution &sol) {
	vector<int> order(p.numJobs);
	for (int i = 0; i<p.numJobs; i++) {
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
	}
	return p.serialSGSWithOvertimeWithForwardBackwardImprovement(order, true).sts;
}

#endif