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



#endif /* NetStructs_h */
