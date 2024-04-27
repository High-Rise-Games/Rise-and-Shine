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


class NetStructs {
    
public:
    
    cugl::physics2::net::LWSerializer _serializer;
    cugl::physics2::net::LWDeserializer _deserializer;
    
    
    enum STRUCT_TYPE {
        DirtRequestType = 1,
        BoardStateType = 2,
        ProjectileType = 3,
        WindowDirtType = 4
    };
    
    enum PROJECTILE_TYPE {
        Poop = 5,
        Dirt = 6
    };
    
    
    struct PROJECTILE {
        float PosX;
        float PosY;
        float velX;
        float velY;
        Sint32 destX;
        Sint32 destY;
        Sint32 type;
    };
    
    struct WINDOW_DIRT {
        float posX;
        float posY;
    };
    
#pragma pack(push, 1)
    struct DIRT_REQUEST {
        
        STRUCT_TYPE type = DirtRequestType;
        Sint32 playerIdSource;
        Sint32 playerIdTarget;
        float dirtPosX;
        float dirtPosY;
        float dirtVelX;
        float dirtVelY;
        Sint32 dirtDestX;
        Sint32 dirtDestY;
        Sint32 dirtAmount;
        
    };
#pragma pack(pop)
    
    struct BOARD_STATE {
        
        STRUCT_TYPE type = BoardStateType;
        bool optional;
        Sint32 numProjectile;
        Sint32 windowDirtAmount;
        Sint32 playerId;
        Sint32 playerChar;
        Sint32 hasWon;
        Sint32 numDirt;
        Sint32 currBoard;
        float playerX;
        float playerY;
        char animState;
        Sint32 timer;
        float birdPosX;
        float birdPosY;
        std::vector<WINDOW_DIRT> dirtVector;
        std::vector<PROJECTILE> projectileVector;
        float progress;
    };
    
    const std::shared_ptr<std::vector<std::byte>> serializeDirtRequest(std::shared_ptr<DIRT_REQUEST> message);
    
    const std::shared_ptr<std::vector<std::byte>> serializeBoardState(std::shared_ptr<NetStructs::BOARD_STATE> message);
    
    const std::shared_ptr<DIRT_REQUEST> deserializeDirtRequest(const std::vector<std::byte>& data);
    
    const std::shared_ptr<BOARD_STATE> deserializeBoardState(const std::vector<std::byte>& data);
    
};

#endif /* NetStructs_h */
