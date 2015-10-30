//
// Created by André Schnabel on 30.10.15.
//

#include "TimeWindow.h"


LambdaBeta TimeWindowBordersGA::init(int ix) {
    return LambdaBeta();
}

void TimeWindowBordersGA::crossover(LambdaBeta &mother, LambdaBeta &father, LambdaBeta &daughter) {
}

void TimeWindowBordersGA::mutate(LambdaBeta &i) {
}

float TimeWindowBordersGA::fitness(LambdaBeta &i) {
    return 0;
}

LambdaTau TimeWindowArbitraryGA::init(int ix) {
    return LambdaTau();
}

void TimeWindowArbitraryGA::crossover(LambdaTau &mother, LambdaTau &father, LambdaTau &daughter) {

}

void TimeWindowArbitraryGA::mutate(LambdaTau &i) {

}

float TimeWindowArbitraryGA::fitness(LambdaTau &i) {
    return 0;
}

vector<int> CompareAlternativesGA::init(int ix) {
    return __1::vector<int>();
}

void CompareAlternativesGA::crossover(vector<int> &mother, vector<int> &father, vector<int> &daughter) {

}

void CompareAlternativesGA::mutate(vector<int> &i) {
    neighborhoodSwap(i);
}

float CompareAlternativesGA::fitness(vector<int> &i) {
    return 0;
}
