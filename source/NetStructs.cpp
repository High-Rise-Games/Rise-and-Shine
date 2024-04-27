//
//  NetStructs.cpp
//  Shine
//
//  Created by Troy Moslemi on 4/26/24.
//



#include <stdio.h>
#include "NetStructs.h"
using namespace cugl;
using namespace std;



const std::shared_ptr<std::vector<std::byte>> NetStructs::serializeDirtRequest(std::shared_ptr<DIRT_REQUEST> message) {
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

const std::shared_ptr<NetStructs::DIRT_REQUEST> NetStructs::deserializeDirtRequest(const std::vector<std::byte>& data) {
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

const std::shared_ptr<std::vector<std::byte>> NetStructs::serializeBoardState(std::shared_ptr<NetStructs::BOARD_STATE> message) {
    _serializer.reset();
    _serializer.writeSint32(message->type);
    _serializer.writeSint32(message->numProjectile);
    _serializer.writeBool(message->optional);
    _serializer.writeSint32(message->playerId);
    _serializer.writeBool(message->hasWon);
    _serializer.writeFloat(message->currBoard);
    _serializer.writeFloat(message->playerY);
    _serializer.writeSint32(message->timer);
    _serializer.writeSint32(message->numDirt);
    _serializer.writeFloat(message->playerY);
    _serializer.writeFloat(message->progress);
    if (!message->optional) {
        _serializer.writeFloat(message->playerX);
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

const std::shared_ptr<NetStructs::BOARD_STATE> NetStructs::deserializeBoardState(const std::vector<std::byte>& data) {
    _deserializer.reset();
    _deserializer.receive(data);
    std::shared_ptr<BOARD_STATE> recievedMessage = std::make_shared<BOARD_STATE>();
    
    
    recievedMessage->type = static_cast<STRUCT_TYPE>(_deserializer.readSint32());
    recievedMessage->numProjectile = _deserializer.readSint32();
    recievedMessage->optional = _deserializer.readBool();
    recievedMessage->playerId = _deserializer.readSint32();
    recievedMessage->hasWon = _deserializer.readBool();
    recievedMessage->currBoard = _deserializer.readFloat();
    recievedMessage->playerY = _deserializer.readFloat();
    recievedMessage->timer = _deserializer.readSint32();
    recievedMessage->numDirt = _deserializer.readSint32();
    recievedMessage->playerY = _deserializer.readFloat();
    recievedMessage->progress = _deserializer.readFloat();
    if (!recievedMessage->optional) {
        recievedMessage->playerX = _deserializer.readFloat();
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
