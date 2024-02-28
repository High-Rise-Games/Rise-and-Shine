//
//  DirtTimer.cpp
//  Ship
//
//  Created by Troy Moslemi on 2/26/24.
//


#include "DirtTimer.h"
#include <stdio.h>
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
#include <random>

bool DirtTimer::init() {
    
    _rng.seed(std::time(nullptr));
    _dirtGenSpeed = 2;
    _dirtThrowTimer = 0;
    _fixedDirtUpdateThreshold = 5 * 60;
    
    return true;
};

/** update when dirt is generated */
void DirtTimer::updateDirtGenTime() {
    _dirtGenTimes.clear();
    std::uniform_int_distribution<> distr(0, _fixedDirtUpdateThreshold);
    for(int n=0; n<_dirtGenSpeed; ++n) {
        _dirtGenTimes.insert(distr(_rng));
    }
}


