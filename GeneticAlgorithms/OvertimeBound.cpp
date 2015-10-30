//
// Created by André Schnabel on 30.10.15.
//

#include "OvertimeBound.h"


LambdaZrt TimeVaryingCapacityGA::init(int ix) {
    return LambdaZrt();
}

void TimeVaryingCapacityGA::crossover(LambdaZrt &mother, LambdaZrt &father, LambdaZrt &daughter) {

}

void TimeVaryingCapacityGA::mutate(LambdaZrt &i) {

}

float TimeVaryingCapacityGA::fitness(LambdaZrt &i) {
    return 0;
}

LambdaZr FixedCapacityGA::init(int ix) {
    return LambdaZr();
}

void FixedCapacityGA::crossover(LambdaZr &mother, LambdaZr &father, LambdaZr &daughter) {

}

void FixedCapacityGA::mutate(LambdaZr &i) {

}

float FixedCapacityGA::fitness(LambdaZr &i) {
    return 0;
}
