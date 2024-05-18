//
//  GLLoadingScene.cpp
//  Ship Lab
//
//  This module provides a very barebones loading screen.  Most of the time you
//  will not need a loading screen, because the assets will load so fast.  But
//  just in case, this is a simple example you can use in your games.
//
//  We know from 3152 that you all like to customize this screen.  Therefore,
//  we have kept it as simple as possible so that it is easy to modify. In
//  fact, this loading screen uses the new modular JSON format for defining
//  scenes.  See the file "loading.json" for how to change this scene.
//
//  Author: Walker White
//  Version: 1/20/22
//
#include "LoadingScene.h"

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
bool LoadingScene::init(const std::shared_ptr<AssetManager>& assets) {
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
    _assets->loadDirectory("json/loading.json");
    _layer = assets->get<scene2::SceneNode>("load");
    _layer->setContentSize(dimen);
    _layer->doLayout(); // This rearranges the children to fit the screen
    
    _bar = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("load_bar"));
    _knob = assets->get<scene2::SceneNode>("load_knob");
    _brand = assets->get<scene2::SceneNode>("load_name");
    _button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("load_play"));
    _button->addListener([=](const std::string& name, bool down) {
        this->_active = down;
    });
    
    _frame_cols = 4;
    _frame_size = 64;
    _curFrame = 0;
    
    _loading_animation_1 = SpriteSheet::alloc(_assets->get<Texture>("loading_1"), 8, 4, _frame_size / 2);
    _loading_animation_1->setFrame(0);
    _loading_animation_2 = SpriteSheet::alloc(_assets->get<Texture>("loading_2"), 8, 4, _frame_size / 2);
    _loading_animation_2->setFrame(0);

    Application::get()->setClearColor(Color4(192,192,192,255));
    addChild(_layer);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LoadingScene::dispose() {
    // Deactivate the button (platform dependent)
    if (isPending()) {
        _button->deactivate();
    }
    _button = nullptr;
    _brand = nullptr;
    _bar = nullptr;
    _assets = nullptr;
    _progress = 0.0f;
}


#pragma mark -
#pragma mark Progress Monitoring
/**
 * The method called to update the game mode.
 *
 * This method updates the progress bar amount.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LoadingScene::update(float progress) {
    if (_progress < 1) {
        _progress = _assets->progress();
        if (_progress >= 1) {
            _progress = 1.0f;
            _bar->setVisible(false);
            _knob->setVisible(false);
            _button->setVisible(true);
            _button->activate();
        }
        _bar->setProgress(_progress);
        _knob->setPositionX(_bar->getPositionX() - (_bar->getWidth() * 0.5) + (_bar->getWidth() * _bar->getProgress()));
        _knob->setPositionY(_bar->getPositionY());
    }
}

void LoadingScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    batch->begin(getCamera()->getCombined());
    _layer->render(batch);
    _curFrame = (int) (cugl::EasingFunction::cubicInOut(_progress) * _frame_size);
    if (_curFrame < 32) {
        //CULog("anim 1 playing frame %f", std::round((_frame_size-1) * (_progress * 2)));
        _loading_animation_1->setFrame(_curFrame);
    } else {
        //CULog("anim 2 playing frame %f", std::round((_frame_size-1) * ((_progress - 0.5) * 2)));
        _loading_animation_1->setFrame(31);
        _loading_animation_2->setFrame(_curFrame % 32);
        if (_curFrame >= 64) {
            _loading_animation_2->setFrame(31);
        }
    }
    Affine2 trans1;
    trans1.scale(0.5);
    trans1.translate(0, getSize().height * 0.5);
    _loading_animation_1->draw(batch, trans1);
    
    Affine2 trans2;
    trans2.scale(0.5);
    trans2.translate(getSize().width * 0.5, getSize().height * 0.5);
    _loading_animation_2->draw(batch, trans2);
    
    batch->end();
}

/**
 * Returns true if loading is complete, but the player has not pressed play
 *
 * @return true if loading is complete, but the player has not pressed play
 */
bool LoadingScene::isPending( ) const {
    return _button != nullptr && _button->isVisible();
}

