//
//  NetStructs.h
//  Shine
//
//  Created by Troy Moslemi on 4/24/24.
//

#ifndef NetStructs_h
#define NetStructs_h
#include <cugl/cugl.h>
#include <stdio.h>
#include <iostream>
using namespace cugl;

enum STRUCT_TYPE {
    DirtRequestType = 1,
    BoardStateType = 2,
    ProjectileType = 3,
};

struct PROJECTILE {
    float PosX;
    float PosY;
    float velX;
    float velY;
    int destX;
    int destY;
    char type;
};

struct DIRT_REQUEST {
    
    STRUCT_TYPE type = DirtRequestType;
    int playerIdSource;
    int playerIdTarget;
    float dirtPosX;
    float dirtPosY;
    float dirtVelX;
    float dirtVelY;
    int dirtDestX;
    int dirtDestY;
    int dirtAmount;

};

struct BOARD_STATE {
    
    
    unsigned char playerId;
    unsigned char playerChar;
    unsigned char hasWon;
    unsigned char numDirt;
    unsigned char currBoard;
    std::optional<float> playerX;
    float playerY;
    std::optional<unsigned char> animState;
    int timer;
    std::optional<float> birdPosX;
    std::optional<float> birdPosY;
    std::optional<std::vector<std::vector<unsigned char>>> dirts;
    std::optional<std::vector<PROJECTILE>> projectileVector;
    float progress;
};

const std::vector<std::byte>& serializeDirtRequest(DIRT_REQUEST message) {
    auto ptr = reinterpret_cast<std::byte*>(&message);
    auto buffer = std::vector<std::byte>(ptr, ptr + sizeof message);
    return buffer;
};

const DIRT_REQUEST deserializeDirtRequest(const std::vector<std::byte>& buffer) {
    DIRT_REQUEST recievedMessage = *reinterpret_cast<const DIRT_REQUEST*>(&buffer[0]);
    return recievedMessage;
};



#endif /* NetStructs_h */
