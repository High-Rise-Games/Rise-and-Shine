//
//  SelectScreen.cpp
//  Ship
//
//  Created by Troy Moslemi on 2/29/24.
//

#include "SelectScreen.h"

using namespace cugl;

/** This is the ideal size of the logo */
#define SCENE_SIZE  1024

#pragma mark -
#pragma mark Constructors

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
bool SelectScreen::init(const std::shared_ptr<AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    // Lock the scene to a reasonable resolution
    if (dimen.width > dimen.height) {
        dimen *= SCENE_SIZE/dimen.width;
    } else {
        dimen *= SCENE_SIZE/dimen.height;
    }
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // IMMEDIATELY load the splash screen assets
    
    _assets = assets;
    _assets->loadDirectory("json/select.json");
    auto layer = assets->get<scene2::SceneNode>("select");
    layer->setContentSize(dimen);
    layer->doLayout(); // This rearranges the children to fit the screen
    
    _host_button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("host_button"));
    _host_button->addListener([=](const std::string& name, bool down) {
        this->_active = down;
    });
    
    Application::get()->setClearColor(Color4(192,192,192,255));
    addChild(layer);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void SelectScreen::dispose() {
    _host_button = nullptr;
    _assets = nullptr;
}


#pragma mark -
#pragma mark Progress Monitoring
/**
 * The method called to update the game mode.
 *
 * This method updates the App state depending on whether player wants
 * to host a game or join a game.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void SelectScreen::update(float progress) {
    
}
