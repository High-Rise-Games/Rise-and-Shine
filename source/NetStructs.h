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
    cugl::net::NetcodeSerializer _serializer;
    
    /* The deserializer */
    cugl::net::NetcodeDeserializer _deserializer;
    
    
    enum STRUCT_TYPE {
        /* Designates the type of the struct as a dirt request message */
        DirtRequestType = 1,
        
        /* Designates the type of the struct as a board state message */
        BoardStateType = 2,
        
        /* Designates the type of the struct as a projectile type message message */
        ProjectileType = 3,
        
        /* Designates the type of the struct as a window dirt type message */
        WindowDirtType = 4,
        
        /* Designates the type of the struct as a dirt state type message */
        DirtStateType = 7,
        
        /* Designates the type of the struct as a dirt state type message */
        MoveStateType = 8,
        
        /* Designates the type of the struct as a scene switch state type message */
        SceneSwitchType = 27
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
        
        /* The x location of the projectile source */
        float SourceX;
        
        /* The y location of the projectile source */
        float SourceY;
        
        /* The x-coordinate of the destination of the projectile */
        float destX;
        
        /* The y-coordinate of the destination of the projectile */
        float destY;
        
        /* The tye of the projectile */
        float type;
    };
    
    struct WINDOW_DIRT {
        
        /* The x-coordinate of the window dirt */
        float posX=0;
        
        /* The y-coordinate of the window dirt */
        float posY=0;
        
        /* Robert's bird poo thingy */
        
        float birdPoo;
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
        float dirtDestX;
        
        /* The y-destination of this dirt */
        float dirtDestY;
        
        /* The amount of dirt that is thrown */
        Sint32 dirtAmount;
        
    };

    struct BOARD_STATE {
        
        /* Sets the default of ths struct as a board state type */
        STRUCT_TYPE type = BoardStateType;
        
        /* Determines whether some of the fields are optional */
        bool optional;
        
        /* The number of projectiles in the board state */
        float numProjectile;
        
        
        /* The player ID that owns this board state */
        float playerId;
        
        /* The player character of the player that owns this board state */
        float playerChar;

        /** Current frame number in the countdown animation spritesheet */
        float countdownFrames;
        
        /* Whether the board state as achieved a win */
        float hasWon;
        
        /* Bool value of whether there is a bird on the board by owner of this board state message */
        bool currBoardBird;
        
        /* The number of dirt that the player has collected in thier bucket*/
        float numDirt;
        
        /* The current board of the player of this board state message */
        float currBoard;
        
        /* The x-coordinate of the player of the board state message */
        float playerX;
        
        /* The y-coordinate of the player of the board state message */
        float playerY;
        
        /* The animation state of the player of this board state message */
        float animState;
        
        /* The time left in the game for this board state message */
        float timer;
        
        /* The x-position of the bird in the board state message */
        float birdPosX=0;
        
        /* The y-position of the bird in the board state message */
        float birdPosY=0;
        
        /** Whether the bird is facing right, needed for bird animation on client side */
        bool birdFacingRight;
        
        /* The vector of PROJECTILE objects */
        std::vector<PROJECTILE> projectileVector;
        
        /* The progress of this board state message */
        float progress;
    };
    
    
    struct DIRT_STATE {
        
        /* Sets the default of ths struct as a board state type */
        STRUCT_TYPE type = DirtStateType;
        
        /* The player ID that owns this board state */
        float playerId;
        
        /* The number of window dirt in this board state message */
        float numWindowDirt;
        
        /* The vector of WINDOW_DIRT objects */
        std::vector<WINDOW_DIRT> dirtVector;
        

    };
    
    struct MOVE_STATE {
        
        /* Sets the default of ths struct as a board state type */
        STRUCT_TYPE type = MoveStateType;
        
        /* The player ID that owns this board state */
        float playerId;
        
        /* The velocity of the player's over request in x direction */
        float moveX;
        
        /* The velocity of the player's over request in y direction */
        float moveY;
        

    };
    
    struct SCENE_SWITCH_STATE {
        
        /* Sets the default of ths struct as a board state type */
        STRUCT_TYPE type = SceneSwitchType;
        
        /* The player ID that owns this scene switch state */
        float playerId;
        
        /* The destination that the owwner of this switch state wants to move to */
        float switchDestination;
        

    };
    

    /* To serialize a DIRT_REQUEST message to send over the network */
    const std::shared_ptr<std::vector<std::byte>> serializeDirtRequest(std::shared_ptr<DIRT_REQUEST> message);
    
    /* To serialize a DIRT_STATE message to send over the network */
    const std::shared_ptr<std::vector<std::byte>> serializeDirtStateMessage(std::shared_ptr<DIRT_STATE> message);
    
    /* To serialize a BOARD_STATE message to send over te network */
    const std::shared_ptr<std::vector<std::byte>> serializeBoardState(std::shared_ptr<NetStructs::BOARD_STATE> message);
    
    /* To serialize a MOVE_STATE message to send over te network */
    const std::shared_ptr<std::vector<std::byte>> serializeMoveState(std::shared_ptr<NetStructs::MOVE_STATE> message);
    
    /* To serialize a SCENE_SWITCH_STATE message to send over te network */
    const std::shared_ptr<std::vector<std::byte>> serializeSwitchState(std::shared_ptr<NetStructs::SCENE_SWITCH_STATE> message);
    
    /* The deserialize a SCENE_SWITCH_STATE message sent over the network */
    const std::shared_ptr<SCENE_SWITCH_STATE> deserializeSwitchState(const std::vector<std::byte>& data);
    
    /* The deserialize a MOVE_STATE message sent over the network */
    const std::shared_ptr<MOVE_STATE> deserializeMoveState(const std::vector<std::byte>& data);
    
    /* The deserialize a DIRT_REQUEST message sent over the network */
    const std::shared_ptr<DIRT_REQUEST> deserializeDirtRequest(const std::vector<std::byte>& data);
    
    /* The deserialize a DIRT_STATE message sent over the network */
    const std::shared_ptr<DIRT_STATE> deserializeDirtStateMessage(const std::vector<std::byte>& data);
    
    /* The deserialize a BOARD_STATE message sent over the network */
    const std::shared_ptr<BOARD_STATE> deserializeBoardState(const std::vector<std::byte>& data);
    
};

#endif /* NetStructs_h */
