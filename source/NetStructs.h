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
    Sint32 animState;
    Sint32 timer;
    float birdPosX;
    float birdPosY;
    std::vector<WINDOW_DIRT> dirtVector;
    std::vector<PROJECTILE> projectileVector;
    float progress;
};

const std::shared_ptr<std::vector<std::byte>> serializeDirtRequest(std::shared_ptr<DIRT_REQUEST> message) {
    _serializer.reset();
    _serializer.writeSint32(message->type);
    _serializer.writeSint32(message->playerIdSource);
    _serializer.writeSint32(message->playerIdTarget);
    _serializer.writeFloat(message->dirtPosX);
    _serializer.writeFloat(message->dirtPosY);
    _serializer.writeFloat(message->dirtVelX);
    _serializer.writeFloat(message->dirtVelY);
    _serializer.writeSint32(message->dirtDestX);
    _serializer.writeSint32(message->dirtDestY);
    _serializer.writeSint32(message->dirtAmount);
    std::shared_ptr<std::vector<std::byte>> buffer = std::make_shared<std::vector<std::byte>>(_serializer.serialize());
    return buffer;
};

const std::shared_ptr<DIRT_REQUEST> deserializeDirtRequest(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    std::shared_ptr<DIRT_REQUEST> recievedMessage = std::make_shared<DIRT_REQUEST>();
    recievedMessage->type = static_cast<STRUCT_TYPE>(_deserializer.readSint32());
    recievedMessage->playerIdSource = _deserializer.readSint32();
    recievedMessage->playerIdTarget = _deserializer.readSint32();
    recievedMessage->dirtPosX = _deserializer.readFloat();
    recievedMessage->dirtPosY = _deserializer.readFloat();
    recievedMessage->dirtVelX = _deserializer.readFloat();
    recievedMessage->dirtVelY = _deserializer.readFloat();
    recievedMessage->dirtAmount = _deserializer.readSint32();
    return recievedMessage;
};

const std::shared_ptr<std::vector<std::byte>> serializeBoardState(std::shared_ptr<BOARD_STATE> message) {
    _serializer.reset();
    _serializer.writeSint32(message->type);
    _serializer.writeSint32(message->numProjectile);
    _serializer.writeSint32(message->optional);
    _serializer.writeSint32(message->playerId);
    _serializer.writeSint32(message->hasWon);
    _serializer.writeFloat(message->currBoard);
    _serializer.writeFloat(message->playerY);
    _serializer.writeSint32(message->timer);
    _serializer.writeSint32(message->numDirt);
    _serializer.writeFloat(message->playerY);
    _serializer.writeFloat(message->progress);
    if (!message->optional) {
        _serializer.writeFloat(message->playerX);
        _serializer.writeSint32(message->animState);
        _serializer.writeFloat(message->birdPosX);
        _serializer.writeFloat(message->birdPosY);
        for (PROJECTILE projectile : message->projectileVector) {
            _serializer.writeFloat(projectile.PosX);
            _serializer.writeFloat(projectile.PosY);
            _serializer.writeFloat(projectile.velX);
            _serializer.writeFloat(projectile.velY);
            _serializer.writeSint32(projectile.destX);
            _serializer.writeSint32(projectile.destY);
            _serializer.writeSint32(projectile.type);
        } for (WINDOW_DIRT dirt : message->dirtVector) {
            _serializer.writeSint32(dirt.posX);
            _serializer.writeSint32(dirt.posY);
        }
    }

    std::shared_ptr<std::vector<std::byte>> buffer = std::make_shared<std::vector<std::byte>>(_serializer.serialize());
    return buffer;
};

const std::shared_ptr<BOARD_STATE> deserializeBoardState(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    std::shared_ptr<BOARD_STATE> recievedMessage = std::make_shared<BOARD_STATE>();
    
    _deserializer.reset();
    recievedMessage->type = static_cast<STRUCT_TYPE>(_deserializer.readSint32());
    recievedMessage->numProjectile = _deserializer.readSint32();
    recievedMessage->optional = _deserializer.readBool();
    recievedMessage->playerId = _deserializer.readSint32();
    recievedMessage->hasWon = _deserializer.readSint32();
    recievedMessage->currBoard = _deserializer.readFloat();
    recievedMessage->playerY = _deserializer.readFloat();
    recievedMessage->timer = _deserializer.readSint32();
    recievedMessage->numDirt = _deserializer.readSint32();
    recievedMessage->playerY = _deserializer.readFloat();
    recievedMessage->progress = _deserializer.readFloat();
    if (!recievedMessage->optional) {
        recievedMessage->playerX = _deserializer.readFloat();
        recievedMessage->animState = _deserializer.readSint32();
        recievedMessage->birdPosX = _deserializer.readSint32();
        recievedMessage->birdPosY = _deserializer.readSint32();
        std::vector<PROJECTILE> projectileVector;
        for (int i=0; i<recievedMessage->numProjectile; i++) {
            PROJECTILE projectile;
            projectile.PosX = _deserializer.readFloat();
            projectile.PosY = _deserializer.readFloat();
            projectile.velX = _deserializer.readFloat();
            projectile.velY = _deserializer.readFloat();
            projectile.destX = _deserializer.readFloat();
            projectile.destY = _deserializer.readFloat();
            projectile.type = static_cast<PROJECTILE_TYPE>(_deserializer.readSint32());
            projectileVector.push_back(projectile);
        } 
        recievedMessage->projectileVector = projectileVector;
        std::vector<WINDOW_DIRT> dirtVector;
        for (int i=0; i<recievedMessage->numDirt; i++) {
            WINDOW_DIRT windowDirt;
            windowDirt.posX = _deserializer.readSint32();
            windowDirt.posY = _deserializer.readSint32();
            dirtVector.push_back(windowDirt);
        }
        recievedMessage->dirtVector = dirtVector;
    }
    
    return recievedMessage;
    
};

//"player_id":  "1",
//"player_char": "Frog",
//"has_won": "false",
//"num_dirt": "1",
//"curr_board": "0",
//"player_x": "3.0",
//"player_y": "4.0",
//"anim_state": "WIPING",
//"timer": "145",
//"bird_pos": ["2.4", "6.0"],
//"bird_facing_right": true,
//"dirts": [ ["0", "1"], ["2", "2"], ["0", "2"] ],
//"projectiles": [
//        {
//            "pos": ["3.0", "1.45"],
//            "vel": ["2", "3"],
//            "dest": ["12.23", "23.5"],
//            "type: "DIRT"
//        },
//        {
//            "pos": ["5.0", "0.2"],
//            "vel": ["0", "-2"],
//            "dest": ["12.23", "23.5"],
//            "type": "POOP"
//        }
//    ]
// }


//* {
//   "player_id":  "1",
//   "player_char": "Frog",
//   "has_won": "false",
//   "num_dirt": "1",
//   "curr_board": "0",
//   "player_y": "4.0",
//   "timer": "145",
//   "progress": "0.7"
//* }

#endif /* NetStructs_h */
