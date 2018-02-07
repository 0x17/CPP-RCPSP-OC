
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
		sgs(ScheduleGenerationScheme::SERIAL),
		rbbrs(false),
		enforceTopOrdering(true),
		fbiFeedbackInjection(false),
		partitionSize(4) {
}

void GAParameters::fromJsonStr(const std::string &s) {
	const auto obj = JsonUtils::readJsonFromString(s);

	const std::map<std::string, int *> keyNamesToIntSlots = {
			{"numGens", &numGens},
			{"popSize", &popSize},
			{"pmutate", &pmutate},
			{"iterLimit", &iterLimit},
			{"partitionSize", &partitionSize}
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

	JsonUtils::assignNumberSlotsFromJsonWithMapping<int>(obj, keyNamesToIntSlots);
	JsonUtils::assignNumberSlotsFromJsonWithMapping<double>(obj, keyNamesToDoubleSlots);
	JsonUtils::assignBooleanSlotsFromJsonWithMapping(obj, keyNamesToBooleanSlots);

	if(obj["selectionMethod"].is_string()) {
		selectionMethod = boost::equals(obj["selectionMethod"].string_value(), "best") ? SelectionMethod::BEST : SelectionMethod::DUEL;
	}

	if(obj["crossoverMethod"].is_string()) {
		crossoverMethod = boost::equals(obj["crossoverMethod"].string_value(), "TPC") ? CrossoverMethod::TPC : CrossoverMethod::OPC;
	}

	if (obj["scheduleGenerationScheme"].is_string()) {
		sgs = boost::equals(obj["scheduleGenerationScheme"].string_value(), "Parallel") ? ScheduleGenerationScheme::PARALLEL : ScheduleGenerationScheme::SERIAL;
	}
}

void GAParameters::fromJsonFile(const std::string &fn) {
	// FIXME: is_regular_file is false on existing file on Mac, why?
	//if(boost::filesystem::exists(fn) && boost::filesystem::is_regular_file(fn))
	if(Utils::fileExists(fn))
		fromJsonStr(Utils::slurp(fn));
}

json11::Json GAParameters::toJson() const {
	return json11::Json::object {
			{"popSize", popSize},
			{"crossoverMethod", crossoverMethod == CrossoverMethod::OPC ? "OPC" : "TPC"},
			{"enforceTopOrdering", enforceTopOrdering},
			{"fbiFeedbackInjection", fbiFeedbackInjection},
			{"fitnessBasedPairing", fitnessBasedPairing},
			{"numGens", numGens},
			{"pmutate", pmutate},
			{"rbbrs", rbbrs},
			{"selectionMethod", selectionMethod == SelectionMethod::BEST ? "best" : "duel"},
			{"scheduleGenerationScheme", sgs == ScheduleGenerationScheme::SERIAL ? "Serial" : "Parallel"},
			{"iterLimit", iterLimit},
			{"outPath", outPath},
			{"threadCount", threadCount},
			{"timeLimit", timeLimit},
			{"traceobj", traceobj},
			{"partitionSize", partitionSize}
	};
}
