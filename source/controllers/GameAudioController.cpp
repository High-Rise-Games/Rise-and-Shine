//
//  GameAudioController.cpp
//  Shine
//
//  Created by Troy Moslemi on 3/18/24.
//

#include "GameAudioController.h"
using namespace cugl;


bool GameAudioController::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    
    _gameplayIsActive = false;
    _gampeplayMusicIsActive = false;
    _assets = assets;
    _gameplayMusic = _assets->get<cugl::Sound>("tower_of_dragons");

    return true;
}

void GameAudioController::playGameplayMusic() {

    AudioEngine::get()->getMusicQueue()->play(_gameplayMusic);
    
};

void GameAudioController::stopGameplayMusic() {
    AudioEngine::get()->getMusicQueue()->clear();
};

void GameAudioController::update(bool gameplayIsActive) {
    if (gameplayIsActive && !getGameplayMusicStatus()) {
        setGameplayMusicActive(true);
        playGameplayMusic();
    } else if (!gameplayIsActive) {
        setGameplayMusicActive(false);
        stopGameplayMusic();
    }
};
