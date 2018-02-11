#include "PartitionModels.h"

using namespace std;
using namespace localsolver;

//======================================================================================================================

PartitionsModel::PartitionsModel(ProjectWithOvertime &_p) : LSBaseModel(_p, new PartitionsSchedulingNativeFunction(_p)), partitionElems(_p.numJobs / options.partitionSize, options.partitionSize) {
	assert(_p.numJobs % options.partitionSize == 0);
}

void PartitionsModel::buildModel(localsolver::LSModel &model, localsolver::LSExpression &obj) {
	partitions = Utils::constructVector<LSExpression>(p.numJobs / options.partitionSize, [&](int pix) { return model.listVar(p.numJobs); });
	for (const auto &partition : partitions) {
		model.constraint(model.count(partition) == options.partitionSize);
	}

	for (int i = 0; i < partitions.size(); i++) {
		for (int j = 0; j<options.partitionSize; j++) {
			partitionElems(i, j) = model.at(partitions[i], j);
			obj.addOperand(partitionElems(i, j));
		}
	}

	model.constraint(model.partition(partitions.begin(), partitions.end()));

	const auto partitionOfJob = [&](int j) {
		LSExpression itspartition = model.sum();
		for (int pix = 0; pix<partitions.size(); pix++)
			itspartition += model.iif(model.contains(partitions[pix], j), 1, 0) * pix;
		return itspartition;
	};

	p.eachJobPairConst([&](int i, int j) {
		if (p.adjMx(i, j)) {
			model.constraint(partitionOfJob(i) < partitionOfJob(j));
		}
	});
}

void PartitionsModel::applyInitialSolution() {
	// initial solution {0,1,2,...,psize},{psize+1,...,2*psize},...
	int ctr = 0;
	for (int i = 0; i < partitions.size(); i++) {
		auto coll = partitions[i].getCollectionValue();
		for (int j = 0; j < options.partitionSize; j++) {
			coll.add(p.topOrder[ctr++]);
		}
	}
}

void PartitionsModel::addAdditionalData(localsolver::LSModel &model, localsolver::LSExpression &obj) {}

int partitionOfJob(const Matrix<int> &partitions, int j) {
	for (int pix = 0; pix<partitions.getM(); pix++) {
		for (int i = 0; i < partitions.getN(); i++) {
			if (partitions(pix, i) == j) {
				return pix;
			}
		}
	}
	return -1;
}

vector<int> PartitionsModel::parseScheduleFromSolution(localsolver::LSSolution &sol) {
	Matrix<int> partitions(p.numJobs / options.partitionSize, options.partitionSize, [&](int i, int j) {
		return sol.getIntValue(partitionElems(i, j));
	});
	const auto partitionList = Utils::constructVector<int>(p.numJobs, [&](int j) { return partitionOfJob(partitions, j); });
	return p.serialOptimalSubSGSWithPartitionListAndFBI(partitionList).sts;
}

SGSResult PartitionsSchedulingNativeFunction::decode(const Matrix<int> &partitions, const localsolver::LSNativeContext &context) {
	const auto partitionList = Utils::constructVector<int>(p.numJobs, [&](int j) { return partitionOfJob(partitions, j); });
	return p.serialOptimalSubSGSWithPartitionListAndFBI(partitionList);
}

boost::optional<SGSResult> PartitionsSchedulingNativeFunction::coreComputation(const localsolver::LSNativeContext &context) {
	const int partitionSize = LSBaseModel::getOptions().partitionSize;
	Matrix<int> partitions(p.numJobs / partitionSize, partitionSize);

	if (context.count() < varCount()) return boost::optional<SGSResult> {};

	for (int i = 0; i < partitions.getM(); i++) {
		for (int j = 0; j<partitions.getN(); j++) {
			partitions(i, j) = static_cast<int>(context.getIntValue(i*partitions.getN() + j));
			if (partitions(i, j) == Project::UNSCHEDULED)
				return boost::optional<SGSResult> {};
		}
	}

	return decode(partitions, context);
}

int PartitionsSchedulingNativeFunction::varCount() {
	return p.numJobs;
}

//======================================================================================================================

ActivityListPartitionsModel::ActivityListPartitionsModel(ProjectWithOvertime& _p, ListSchedulingNativeFunction *_func) : ListModel(_p, _func) {}
ActivityListPartitionsModel::ActivityListPartitionsModel(ProjectWithOvertime& _p) : ListModel(_p, new ActivityListPartitionsSchedulingNativeFunction(_p)) {}

int ActivityListPartitionsModel::ActivityListPartitionsSchedulingNativeFunction::varCount() {
	return p.numJobs;
}

SGSResult ActivityListPartitionsModel::ActivityListPartitionsSchedulingNativeFunction::decode(vector<int>& order, const LSNativeContext& context) {
	return p.serialOptimalSubSGSAndFBI(order, options.partitionSize, true);
}

void ActivityListPartitionsModel::addAdditionalData(LSModel &model, LSExpression& obj) {}

vector<int> ActivityListPartitionsModel::parseScheduleFromSolution(LSSolution& sol) {
	vector<int> order(p.numJobs);
	for (int i = 0; i<p.numJobs; i++)
		order[i] = static_cast<int>(sol.getIntValue(listElems[i]));
	return p.serialOptimalSubSGSAndFBI(order, options.partitionSize, true).sts;
}