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
    
    /* The serializer */
    cugl::physics2::net::LWSerializer _serializer;
    
    /* The deserializer */
    cugl::physics2::net::LWDeserializer _deserializer;
    
    
    enum STRUCT_TYPE {
        /* Designates the type of the struct as a dirt request message */
        DirtRequestType = 1,
        
        /* Designates the type of the struct as a board state message */
        BoardStateType = 2,
        
        /* Designates the type of the struct as a projectile type message message */
        ProjectileType = 3,
        
        /* Designates the type of the struct as a window dirt type message */
        WindowDirtType = 4
    };
    
    enum PROJECTILE_TYPE {
        
        /* Designates the projectile as poop */
        Poop = 5,
        
        /* Designates the projectile as dirt */
        Dirt = 6
    };
    
    
    struct PROJECTILE {
        
        /* The x coordinate of the projectile */
        float PosX;
        
        /* The y coordinate of the projectile */
        float PosY;
        
        /* The x velocity of the projectile */
        float velX;
        
        /* The y velocity of the projectile */
        float velY;
        
        /* The x-coordinate of the destination of the projectile */
        Sint32 destX;
        
        /* The y-coordinate of the destination of the projectile */
        Sint32 destY;
        
        /* The tye of the projectile */
        Sint32 type;
    };
    
    struct WINDOW_DIRT {
        
        /* The x-coordinate of the window dirt */
        float posX=0;
        
        /* The y-coordinate of the window dirt */
        float posY=0;
    };
    

    struct DIRT_REQUEST {
        
        /* We set the default type of this message as a dirt request message */
        STRUCT_TYPE type = DirtRequestType;
        
        /* The player ID that sent over the dirt request message */
        Sint32 playerIdSource;
        
        /* The player ID that we send this struct to */
        Sint32 playerIdTarget;
        
        /* The x-coordinate of this dirt */
        float dirtPosX;
        
        /* The y-coordinate of this dirt */
        float dirtPosY;
        
        /* The x-velocity of this dirt */
        float dirtVelX;
        
        /* The y-velocity of this dirt */
        float dirtVelY;
        
        /* The x-destination of this dirt */
        Sint32 dirtDestX;
        
        /* The y-destination of this dirt */
        Sint32 dirtDestY;
        
        /* The amount of dirt that is thrown */
        Sint32 dirtAmount;
        
    };

    struct BOARD_STATE {
        
        /* Sets the default of ths struct as a board state type */
        STRUCT_TYPE type = BoardStateType;
        
        /* Determines whether some of the fields are optional */
        bool optional;
        
        /* The number of projectiles in the board state */
        Sint32 numProjectile;
        
        /* The number of window dirts in the board state */
        Sint32 windowDirtAmount;
        
        /* The player ID that owns this board state */
        Sint32 playerId;
        
        /* The player character of the player that owns this board state */
        Sint32 playerChar;
        
        /* Whether the board state as achieved a win */
        Sint32 hasWon;
        
        /* The number of dirt that the player has collected in thier bucket*/
        Sint32 numDirt;
        
        /* The current board of the player of this board state message */
        Sint32 currBoard;
        
        /* The x-coordinate of the player of the board state message */
        float playerX;
        
        /* The y-coordinate of the player of the board state message */
        float playerY;
        
        /* The animation state of the player of this board state message */
        Sint32 animState;
        
        /* The time left in the game for this board state message */
        Sint32 timer;
        
        /* The x-position of the bird in the board state message */
        float birdPosX=0;
        
        /* The number of window dirt in this board state message */
        Sint32 numWindowDirt;
        
        /* The y-position of the bird in the board state message */
        float birdPosY=0;
        
        /* The vector of WINDOW_DIRT objects */
        std::vector<WINDOW_DIRT> dirtVector;
        
        /* The vector of PROJECTILE objects */
        std::vector<PROJECTILE> projectileVector;
        
        /* The progress of this board state message */
        float progress;
    };
    

    /* To serialize a DIRT_REQUEST message to send over the network */
    const std::shared_ptr<std::vector<std::byte>> serializeDirtRequest(std::shared_ptr<DIRT_REQUEST> message);
    
    /* To serialize a BOARD_STATE message to send over te network */
    const std::shared_ptr<std::vector<std::byte>> serializeBoardState(std::shared_ptr<NetStructs::BOARD_STATE> message);
    
    /* The deserialize a DIRT_REQUEST message sent over the network */
    const std::shared_ptr<DIRT_REQUEST> deserializeDirtRequest(const std::vector<std::byte>& data);
    
    /* The deserialize a BOARD_STATE message sent over the network */
    const std::shared_ptr<BOARD_STATE> deserializeBoardState(const std::vector<std::byte>& data);
    
};

#endif /* NetStructs_h */
