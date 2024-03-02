//
//  SelectScreen.hpp
//  Ship
//
//  Created by Troy Moslemi on 2/29/24.
//

#ifndef SelectScreen_hpp
#define SelectScreen_hpp

#include <stdio.h>
#include <cugl/cugl.h>



class SelectScreen : public cugl::Scene2 {
protected:
    
    /** The asset manager for ScreenSELECT. */
    std::shared_ptr<cugl::AssetManager> _assets;


    // button to host game
    std::shared_ptr<cugl::scene2::Button>    _host_button;
    
    // button to join a game
    std::shared_ptr<cugl::scene2::Button>    _client_button;

    // MODEL
    /** Whether or not the player has pressed play to continue */
    bool  _clicked_host;

    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a select screen mode for the player to decide between hosting or
     * joing a game.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    SelectScreen() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~SelectScreen() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose();
    
    /**
     * Initializes the controller contents, making it ready for loading
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    
#pragma mark -
#pragma mark Join or Host Game Monitoring
    /**
     * The method called to update the game mode.
     *
     * This method updates the App state depending on whether player wants
     * to host a game or join a game.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void update(float timestep);

};

#endif /* SelectScreen_hpp */
