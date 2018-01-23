
#include "GeneticAlgorithm.h"

FitnessResult::FitnessResult(float value, int numSchedulesGenerated)
		: value(value),
		  numSchedulesGenerated(numSchedulesGenerated) {}

FitnessResult::FitnessResult() : value(0.0f), numSchedulesGenerated(0) {}

GAParameters::GAParameters() :
		BasicSolverParameters(-1.0, -1, false, "GATrace_", 1),
		numGens(200),
		popSize(100),
		pmutate(5),
		fitnessBasedPairing(false),
		selectionMethod(SelectionMethod::BEST),
		crossoverMethod(CrossoverMethod::OPC),
		rbbrs(false),
		enforceTopOrdering(true),
		fbiFeedbackInjection(false) {
}

template<class T>
void assignNumberSlotsFromJsonWithMapping(const json11::Json &obj, const std::map<std::string, T *> mapping) {
	for(const auto &pair : mapping) {
		if(obj[pair.first].is_number()) {
			*pair.second = static_cast<T>(obj[pair.first].number_value());
		}
	}
}

void assignBooleanSlotsFromJsonWithMapping(const json11::Json &obj, const std::map<std::string, bool *> mapping) {
	for (const auto &pair : mapping) {
		if (obj[pair.first].is_bool()) {
			*pair.second = obj[pair.first].bool_value();
		}
	}
}

void GAParameters::fromJsonStr(const std::string &s) {
	const auto obj = JsonUtils::readJsonFromString(s);

	const std::map<std::string, int *> keyNamesToIntSlots = {
			{"numGens", &numGens},
			{"popSize", &popSize},
			{"pmutate", &pmutate},
			{"iterLimit", &iterLimit}
	};

	const std::map<std::string, double *> keyNamesToDoubleSlots = {
			{"timeLimit", &timeLimit}
	};

	const std::map<std::string, bool *> keyNamesToBooleanSlots = {
			{"fitnessBasedPairing", &fitnessBasedPairing},
			{"enforceTopOrdering", &enforceTopOrdering},
			{"rbbrs", &rbbrs},
			{"fbiFeedbackInjection", &fbiFeedbackInjection}
	};

	assignNumberSlotsFromJsonWithMapping<int>(obj, keyNamesToIntSlots);
	assignNumberSlotsFromJsonWithMapping<double>(obj, keyNamesToDoubleSlots);
	assignBooleanSlotsFromJsonWithMapping(obj, keyNamesToBooleanSlots);

	if(obj["selectionMethod"].is_string()) {
		selectionMethod = boost::equals(obj["selectionMethod"].string_value(), "best") ? SelectionMethod::BEST : SelectionMethod::DUEL;
	}

	if(obj["selectionMethod"].is_string()) {
		crossoverMethod = boost::equals(obj["crossoverMethod"].string_value(), "TPC") ? CrossoverMethod::TPC : CrossoverMethod::OPC;
	}
}

void GAParameters::fromJsonFile(const std::string &fn) {
	if(boost::filesystem::exists(fn) && boost::filesystem::is_regular_file(fn))
		fromJsonStr(Utils::slurp(fn));
}
