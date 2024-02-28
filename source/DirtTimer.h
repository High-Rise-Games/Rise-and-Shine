//
//  dirtTimer.h
//  Ship
//
//  Created by Troy Moslemi on 2/26/24.
//

#ifndef dirtTimer_h
#define dirtTimer_h
#include <cugl/cugl.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <random>

class DirtTimer {
private:

    /** Random number generator for dirt generation */
    std::mt19937 _rng;
    
    /** Dirt random generation time stamp*/
    std::set<int> _dirtGenTimes;
    
    /** Timer threshold for fixed period random dirt generation in frames. E.g. 300 is one dirt generation per 5 seconds */
    int _fixedDirtUpdateThreshold;
    
    /** Current timer value for dirt regeneration. Increments up to _fixedDirtUpdateThreshold and resets to 0*/
    int _dirtThrowTimer;
    
    /** Dirt generation speed, equals number of random dirt generated per _fixedDirtUpdateThreshold period*/
    int _dirtGenSpeed;

#pragma mark Input Control
public:
    
    /** update when dirt is generated */
    void updateDirtGenTime();
    
    
    
    bool init();


};



#endif /* dirtTimer_h */
