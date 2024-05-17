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
    
    // Resets the serializer in order for it to be used again
    _serializer.reset();
    
    // Writes the data in a specific order
    _serializer.writeFloat(message->type);
    _serializer.writeFloat(message->playerIdSource);
    _serializer.writeFloat(message->playerIdTarget);
    _serializer.writeFloat(message->dirtPosX);
    _serializer.writeFloat(message->dirtPosY);
    _serializer.writeFloat(message->dirtVelX);
    _serializer.writeFloat(message->dirtVelY);
    _serializer.writeFloat(message->dirtDestX);
    _serializer.writeFloat(message->dirtDestY);
    _serializer.writeFloat(message->dirtAmount);
    
    // Call the serialize() method of _serializer to store the serialized data in buffer
    std::shared_ptr<std::vector<std::byte>> buffer = std::make_shared<std::vector<std::byte>>(_serializer.serialize());
    return buffer;
};

const std::shared_ptr<NetStructs::DIRT_REQUEST> NetStructs::deserializeDirtRequest(const std::vector<std::byte>& data) {
    
    // Resets the deserializer in order for it to be used again
    _deserializer.reset();
    
    // The data gets inserted into the deserializer
    _deserializer.receive(data);
    
    // Create a new DIRT_REQUEST object to insert the deserialized data into
    std::shared_ptr<DIRT_REQUEST> recievedMessage = std::make_shared<DIRT_REQUEST>();
    
    // Deserialize the data in the order it was serialized
    recievedMessage->type = static_cast<STRUCT_TYPE>(_deserializer.readFloat());
    recievedMessage->playerIdSource = _deserializer.readFloat();
    recievedMessage->playerIdTarget = _deserializer.readFloat();
    recievedMessage->dirtPosX = _deserializer.readFloat();
    recievedMessage->dirtPosY = _deserializer.readFloat();
    recievedMessage->dirtVelX = _deserializer.readFloat();
    recievedMessage->dirtVelY = _deserializer.readFloat();
    recievedMessage->dirtDestX = _deserializer.readFloat();
    recievedMessage->dirtDestY = _deserializer.readFloat();
    recievedMessage->dirtAmount = _deserializer.readFloat();
    return recievedMessage;
};

const std::shared_ptr<std::vector<std::byte>> NetStructs::serializeBoardState(std::shared_ptr<NetStructs::BOARD_STATE> message) {
    
    // Resets the serializer in order for it to be used again
    _serializer.reset();
    
    // Writes the data in a specific order
    _serializer.writeFloat(message->type);
    _serializer.writeFloat(static_cast<float>(message->optional));
    _serializer.writeFloat(message->hasWon);
    _serializer.writeFloat(message->numProjectile);
    _serializer.writeFloat(message->playerChar);
    _serializer.writeFloat(message->animState);
    _serializer.writeFloat(message->countdownFrames);
    _serializer.writeFloat(message->playerId);
    _serializer.writeFloat(message->currBoard);
    _serializer.writeFloat(message->playerY);
    _serializer.writeFloat(message->timer);
    _serializer.writeFloat(message->numDirt);
    _serializer.writeFloat(message->progress);
    _serializer.writeFloat(static_cast<float>(message->currBoardBird));
    
    // check if message is optional. If not, write more data
    if (!message->optional) {
        
        _serializer.writeFloat(message->playerX);
        if (message->currBoardBird) {
            _serializer.writeFloat(message->birdPosX);
            _serializer.writeFloat(message->birdPosY);
            _serializer.writeFloat(static_cast<float>(message->birdFacingRight));
        }
        
        
        // loop through every projectile in the projectile vector
        // to write the data into the serializer
        for (PROJECTILE projectile : message->projectileVector) {
            _serializer.writeFloat(projectile.PosX);
            _serializer.writeFloat(projectile.PosY);
            _serializer.writeFloat(projectile.velX);
            _serializer.writeFloat(projectile.velY);
            _serializer.writeFloat(projectile.destX);
            _serializer.writeFloat(projectile.destY);
            _serializer.writeFloat(projectile.type);
            
            // loop through every window in the window vector
            // to write the data into the serializer
        }
    }
    
    
    

    std::shared_ptr<std::vector<std::byte>> buffer = std::make_shared<std::vector<std::byte>>(_serializer.serialize());
    return buffer;
};

const std::shared_ptr<NetStructs::BOARD_STATE> NetStructs::deserializeBoardState(const std::vector<std::byte>& data) {
    
    // Resets the deserializer in order for it to be used again
    _deserializer.reset();
    
    // The data gets inserted into the deserializer
    _deserializer.receive(data);
    
    // Creates a new BOARD_STATE object to write the deserialized data into
    std::shared_ptr<BOARD_STATE> receivedMessage = std::make_shared<BOARD_STATE>();
    
    // Deserialize the data in the order it was serialized
    receivedMessage->type = static_cast<STRUCT_TYPE>(_deserializer.readFloat());
    receivedMessage->optional = static_cast<bool>(_deserializer.readFloat());
    receivedMessage->hasWon = _deserializer.readFloat();
    receivedMessage->numProjectile = _deserializer.readFloat();
    receivedMessage->playerChar = _deserializer.readFloat();
    receivedMessage->animState = _deserializer.readFloat();
    receivedMessage->countdownFrames = _deserializer.readFloat();
    receivedMessage->playerId = _deserializer.readFloat();
    receivedMessage->currBoard = _deserializer.readFloat();
    receivedMessage->playerY = _deserializer.readFloat();
    receivedMessage->timer = _deserializer.readFloat();
    receivedMessage->numDirt = _deserializer.readFloat();
    receivedMessage->progress = _deserializer.readFloat();
    receivedMessage->currBoardBird = static_cast<bool>(_deserializer.readFloat());
    if (!receivedMessage->optional) {
        receivedMessage->playerX = _deserializer.readFloat();
        if (receivedMessage->currBoardBird) {
            receivedMessage->birdPosX = _deserializer.readFloat();
            receivedMessage->birdPosY = _deserializer.readFloat();
            receivedMessage->birdFacingRight = static_cast<bool>(_deserializer.readFloat());
        }
        std::vector<PROJECTILE> projectileVector;
        for (int i=0; i<receivedMessage->numProjectile; i++) {
            PROJECTILE projectile;
            projectile.PosX = _deserializer.readFloat();
            projectile.PosY = _deserializer.readFloat();
            projectile.velX = _deserializer.readFloat();
            projectile.velY = _deserializer.readFloat();
            projectile.destX = _deserializer.readFloat();
            projectile.destY = _deserializer.readFloat();
            projectile.type = static_cast<PROJECTILE_TYPE>(_deserializer.readFloat());
            projectileVector.push_back(projectile);
        }
        receivedMessage->projectileVector = projectileVector;
    }
    
    
    
    
    return receivedMessage;
    
};

const std::shared_ptr<std::vector<std::byte>> NetStructs::serializeDirtStateMessage(std::shared_ptr<NetStructs::DIRT_STATE> message) {
    
    // Resets the serializer in order for it to be used again
    _serializer.reset();
    
    _serializer.writeFloat(message->type);
    _serializer.writeFloat(message->numWindowDirt);
    _serializer.writeFloat(message->playerId);
    
    for (WINDOW_DIRT dirt : message->dirtVector) {
        _serializer.writeFloat(dirt.posX);
        _serializer.writeFloat(dirt.posY);
    }
    

    std::shared_ptr<std::vector<std::byte>> buffer = std::make_shared<std::vector<std::byte>>(_serializer.serialize());
    return buffer;
};

const std::shared_ptr<NetStructs::DIRT_STATE> NetStructs::deserializeDirtStateMessage(const std::vector<std::byte> &data) {
    
    // Resets the deserializer in order for it to be used again
    _deserializer.reset();
    
    // The data gets inserted into the deserializer
    _deserializer.receive(data);
    
    // Creates a new BOARD_STATE object to write the deserialized data into
    std::shared_ptr<DIRT_STATE> recievedMessage = std::make_shared<DIRT_STATE>();
    
    recievedMessage->type = static_cast<STRUCT_TYPE>( _deserializer.readFloat());
    recievedMessage->numWindowDirt =  _deserializer.readFloat();
    recievedMessage->playerId = _deserializer.readFloat();
    
    std::vector<WINDOW_DIRT> dirtVector;
    for (int i=0; i<recievedMessage->numWindowDirt; i++) {
        WINDOW_DIRT dirt;
        dirt.posX = _deserializer.readFloat();
        dirt.posY = _deserializer.readFloat();
        dirtVector.push_back(dirt);
    }
    
    recievedMessage->dirtVector = dirtVector;
    
    return recievedMessage;
    
};

const std::shared_ptr<std::vector<std::byte>> NetStructs::serializeMoveState(std::shared_ptr<NetStructs::MOVE_STATE> message) {
    
    // Resets the serializer in order for it to be used again
    _serializer.reset();
    
    _serializer.writeFloat(message->type);
    _serializer.writeFloat(message->playerId);
    _serializer.writeFloat(message->moveX);
    _serializer.writeFloat(message->moveY);
    

    std::shared_ptr<std::vector<std::byte>> buffer = std::make_shared<std::vector<std::byte>>(_serializer.serialize());
    return buffer;
};

const std::shared_ptr<NetStructs::MOVE_STATE> NetStructs::deserializeMoveState(const std::vector<std::byte> &data) {
    
    // Resets the deserializer in order for it to be used again
    _deserializer.reset();
    
    // The data gets inserted into the deserializer
    _deserializer.receive(data);
    
    // Creates a new BOARD_STATE object to write the deserialized data into
    std::shared_ptr<MOVE_STATE> recievedMessage = std::make_shared<MOVE_STATE>();
    
    recievedMessage->type = static_cast<STRUCT_TYPE>( _deserializer.readFloat());
    recievedMessage->playerId = _deserializer.readFloat();
    recievedMessage->moveX = _deserializer.readFloat();
    recievedMessage->moveY = _deserializer.readFloat();
    
    
    return recievedMessage;
    
};

const std::shared_ptr<std::vector<std::byte>> NetStructs::serializeSwitchState(std::shared_ptr<NetStructs::SCENE_SWITCH_STATE> message) {
    
    // Resets the serializer in order for it to be used again
    _serializer.reset();
    
    _serializer.writeFloat(message->type);
    _serializer.writeFloat(message->playerId);
    _serializer.writeFloat(message->switchDestination);
    

    std::shared_ptr<std::vector<std::byte>> buffer = std::make_shared<std::vector<std::byte>>(_serializer.serialize());
    return buffer;
};

const std::shared_ptr<NetStructs::SCENE_SWITCH_STATE> NetStructs::deserializeSwitchState(const std::vector<std::byte> &data) {
    
    // Resets the deserializer in order for it to be used again
    _deserializer.reset();
    
    // The data gets inserted into the deserializer
    _deserializer.receive(data);
    
    // Creates a new BOARD_STATE object to write the deserialized data into
    std::shared_ptr<SCENE_SWITCH_STATE> recievedMessage = std::make_shared<SCENE_SWITCH_STATE>();
    
    recievedMessage->type = static_cast<STRUCT_TYPE>( _deserializer.readFloat());
    recievedMessage->playerId = _deserializer.readFloat();
    recievedMessage->switchDestination = _deserializer.readFloat();
    
    
    return recievedMessage;
    
};
