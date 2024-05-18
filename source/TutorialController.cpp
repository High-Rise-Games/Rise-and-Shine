//  TutorialController.cpp
//
//  This is the primary class file for running the tutorial.
//
//  Author: High Rise Games
//
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
#include <random>

#include "TutorialController.h"


using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

// Lock the screen size to fixed height regardless of aspect ratio
#define SCENE_HEIGHT 720

#pragma mark -
#pragma mark Constructors

bool TutorialController::initLevel(int selected_level) {
    hostReset();

    // Initialize the tutorial stage system to the first phase
    _currentTutorialStage = MOVE;

    // Get texts for each phase of tutorial
    _TutorialTextBlank = _assets->get<Texture>("TutorialTextBlank");
    vector<string> tutorialTextStrings = { "TutorialText0", "TutorialText1", "TutorialText2", "TutorialText3", "TutorialText4", "TutorialText5", "TutorialText6", "TutorialText7", "TutorialText8" };
    for (string tutorialTextImage : tutorialTextStrings) {
        _TutorialTexts.push_back(_assets->get<Texture>(tutorialTextImage));
    }
    _TutorialDragFinger = _assets->get<Texture>("TutorialDragFinger");

    // TODO: update depending on level
    _birdActive = true;

    // Initialize the window grids
    _levelJson = _assets->get<JsonValue>("tutoriallevel");
    _size = _nativeSize;
    _size.height *= 1.5;

    // texture mappings for level (update these from the python script)
    _texture_strings_selected = { "level1Window1", "level1Window2", "fully_blocked_1", "fully_blocked_2", "fully_blocked_3", "fully_blocked_4", "left_blocked_1", "down_blocked_1", "planter-brown1", "tutorialBuilding" };
    _texture_ids_selected = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    _dirtTextureString = "level1dirt";

    // TODO: update??
    _initDirtCount = selected_level * 5;

    // get the win background when game is win
    _winBackground = _assets->get<Texture>("win-background");

    // get the lose background when game is lose
    _loseBackground = _assets->get<Texture>("lose-background");

    // get the asseets for countdown
    setCountdown1Texture(_assets->get<Texture>("C1"));
    setCountdown2Texture(_assets->get<Texture>("C2"));
    setCountdown3Texture(_assets->get<Texture>("C3"));
    setCountdownGoTexture(_assets->get<Texture>("Go"));
    setCountdownSparkleTexture(_assets->get<Texture>("Sparkle"));

    // Initialize random dirt generation
    // TODO: decide if we still need?
    updateDirtGenTime();

    _collisions.init(_size);

    // Get the bang sound
    _bang = _assets->get<Sound>("bang");
    _clean = _assets->get<Sound>("clean");

    return true;
}

/**
* Initializes the extra controllers needed for the host of the game.
*
* The constructor does not allocate any objects or memory.  This allows
* us to have a non-pointer reference to this controller, reducing our
* memory allocation.  Instead, allocation happens in this method.
*
* Assigns player ids clockwise with host at top
*
*          host: 1
* left: 4            right: 2
*         across: 3
*
* @param assets    The (loaded) assets for this game mode
* @param nPlayers  The number of players for this game
*
* @return true if the controller is initialized properly, false otherwise.
*/
bool TutorialController::initHost(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    }
    
    _ishost=true;

    _numPlayers = 2;

    for (int i = 1; i <= _numPlayers; i++) {
        // Initialize window grids
        _windowVec[i - 1] = make_shared<WindowGrid>();
        for (string thisWindow : _texture_strings_selected) {
            _windowVec[i - 1]->addTexture(_assets->get<Texture>(thisWindow));
        }
        _windowVec[i - 1]->setTextureIds(_texture_ids_selected);
        _windowVec[i - 1]->init(_levelJson, _size); // init depends on texture
        _windowVec[i - 1]->setInitDirtNum(_initDirtCount);
        _windowVec[i - 1]->setDirtTexture(assets->get<Texture>(_dirtTextureString));
        _windowVec[i - 1]->setPooTexture(_assets->get<Texture>("birdPoop"));
        _windowVec[i - 1]->setFadedDirtTexture(assets->get<Texture>("faded-dirt"));
        //_windowVec[i - 1]->generateInitialBoard(_windowVec[i - 1]->getInitDirtNum());
        // generate fixed dirt
        for (int j = 0; j < this->dirt_x_values.size(); j++) {
            //std::shared_ptr<StaticFilth> dirt = std::make_shared<StaticFilth>(Vec2(dirt_x_values.at(j), dirt_y_values.at(j)));
            _windowVec[i - 1]->addDirt(dirt_y_values.at(j), dirt_x_values.at(j), 0);
        }

        // Initialize player characters
        Vec2 startingPos = Vec2(_windowVec[i - 1]->sideGap + (_windowVec[i - 1]->getPaneWidth() / 2), _windowVec[i - 1]->getPaneHeight() / 2);
        _playerVec[i - 1] = make_shared<Player>(i, startingPos, _windowVec[i - 1]->getPaneHeight(), _windowVec[i - 1]->getPaneWidth());
        _playerVec[i - 1]->setPosition(startingPos);
        _playerVec[i - 1]->setVelocity(Vec2::ZERO);
        _playerVec[i - 1]->setAnimationState(Player::AnimStatus::IDLE);

        // Initialize projectiles
        _projectileVec[i - 1] = make_shared<ProjectileSet>();
        _projectileVec[i - 1]->setDirtTexture(assets->get<Texture>(_dirtTextureString));
        _projectileVec[i - 1]->setPoopInFlightTexture(assets->get<Texture>("pooMiddle"));
        _projectileVec[i - 1]->setTextureScales(_windowVec[i - 1]->getPaneHeight(), _windowVec[i - 1]->getPaneWidth());
    }

    // Initialize bird textures, but do not set a location yet. that is the host's job
    if (_birdActive) {
        int height = _windowVec[_id - 1]->getNVertical();
        int width = _windowVec[_id - 1]->getNHorizontal();
        cugl::Vec2 birdTopLeftPos = cugl::Vec2(0.4, height - 0.5);
        cugl::Vec2 birdTopRightPos = cugl::Vec2(width - 0.6, height - 0.5);
        cugl::Vec2 birdBotLeftPos = cugl::Vec2(0.4, height - 3.5);
        cugl::Vec2 birdBotRightPos = cugl::Vec2(width - 0.6, height - 3.5);
        std::vector<cugl::Vec2> positions = { birdTopLeftPos, birdTopRightPos, birdBotLeftPos, birdBotRightPos };
        _bird.init(positions, 0.01, 0.04, _windowVec[_id - 1]->getPaneHeight());
        _bird.setTexture(_assets->get<Texture>("bird"));
        // start with bird on no one's board
        _curBirdBoard = 0;
    }

    return true;
}


#pragma mark Graphics


#pragma mark Gameplay Handling

/**
 * Resets the status of the game for all players so that we can play again.
 */
void TutorialController::hostReset() {
    reset();

    _allDirtAmounts = { 0, 0, 0, 0 };
    _hasWon = { false, false, false, false };
}

/**
* Sets the character of your player (Mushroom) and your opponent
* (Flower) for the tutorial.
*/
void TutorialController::setCharacters(std::vector<std::string>& chars) {
    changeCharTexture(_playerVec[0], "Mushroom");
    _playerVec[0]->setChar("Mushroom");

    changeCharTexture(_playerVec[1], "Flower");
    _playerVec[1]->setChar("Flower");
}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * We need to update this method to constantly talk to the server.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 * @param worldPos  The position of the user's touch in world positions, used for dirt throwing
 * @param dirtCon   The dirt throw input controller used by the game scene
 * @param dirtThrowButton   The dirt throw button from the game scene
 */
void TutorialController::update(float timestep, Vec2 worldPos, DirtThrowInputController& dirtCon, std::shared_ptr<cugl::scene2::Button> dirtThrowButton, std::shared_ptr<cugl::scene2::SceneNode> dirtThrowArc) {

    //CULog("%d", _currentTutorialStage); // TODO: delete me

    // update tutorial phases that aren't easily hooked elsewhere
    if (_currentTutorialStage == FIRST_DIRT && _windowVec[0]->getTotalDirt() < 5 ) {
        _currentTutorialStage = THIRD_DIRT;
    }
    if (_currentTutorialStage == THIRD_DIRT && _windowVec[0]->getTotalDirt() <= 2) {
        _currentTutorialStage = BIRD; // TODO: change to BIRD
        _curBirdBoard = 1;
        int spawnRow = 5;
        _bird.resetBirdPath(_windowVec[0]->getNVertical(), _windowVec[0]->getNHorizontal(), spawnRow);
        _birdLeaving = false;
    }
    if (_currentTutorialStage == BIRD && _curBirdBoard != 1) {
        _currentTutorialStage = PEEK;
    }
    if (_currentTutorialStage == ALL_DIRT && _windowVec[0]->getTotalDirt() == 0) {
        _currentTutorialStage = DONE;
    }

    _input.update();

    // update time
    if (_gameTimeLeft >= 1 && _gameStart) {
        _frame = _frame + 1;
    } if (_frame == _fps) {
        _gameTimeLeft = max(0, _gameTimeLeft - 1);
        _projectileGenChance = 0.95 / (1 + exp(-0.05 * (100 - _gameTimeLeft / 2)));
        _frame = 0;
    }

    // check if game is over when timer hits 0
    if (_gameTimeLeft == 0 && !_gameOver) {
        _gameOver = true;
        vector<int> v;
        for (auto window : _windowVec) {
            if (window == nullptr) continue;
            v.push_back(window->getTotalDirt());
        }
        auto min_idx = std::distance(v.begin(), std::min_element(v.begin(), v.end()));
        _hasWon[min_idx] = true;
    }

    // update bird if active
    if (_birdActive && _curBirdBoard != 0 && _gameStart) {
        _bird.move();
        std::shared_ptr<WindowGrid> windows = _windowVec[_curBirdBoard - 1];
        std::shared_ptr<ProjectileSet> projectiles = _projectileVec[_curBirdBoard - 1];

        if (!_birdLeaving && _bird.atColCenter(windows->getNHorizontal(), windows->getPaneWidth(), windows->sideGap) >= 0) {
            std::bernoulli_distribution dist(_projectileGenChance);
            if (dist(_rng)) {
                // random chance to generate bird poo at column center
                generatePoo(projectiles, windows);
            }
        }
    }

    if (!_gameStart) {
        advanceCountDownAnim();
    }
    else {
        for (int i = 0; i < _numPlayers; i++) {
            stepForward(_playerVec[i], _windowVec[i], _projectileVec[i]);
            if (_currentTutorialStage < DONE) { // don't allow game to end until all stages complete
                _gameOver = false;
                _hasWon[i] = false;
            }
        }
    }

    // _input.update();
    if (_input.didPressReset()) {
        // host resets game for all players
        hostReset();
    }
    // update the game state for self (host). Updates for the rest of the players are done in processMovementRequest(),
    // called whenever the host recieves a movement or other action message.
    _currentDirtAmount = _allDirtAmounts[0];
    _curBirdPos = getWorldPosition(_bird.birdPosition);


    // When the player is on other's board and are able to throw dirt
    int myCurBoard = _allCurBoards[_id - 1];
    if (myCurBoard != 0) {
        bool ifSwitch = false;
        float button_x = myCurBoard == 1 ? getSize().width - _windowVec[_id - 1]->sideGap + 150 : _windowVec[_id - 1]->sideGap - 150;
        float arc_start = myCurBoard == 1 ? 270 : 90;
        float arc_rotate_angle = myCurBoard == 1 ? 0 : M_PI;
        cugl::Vec2 buttonPos(button_x, SCENE_HEIGHT / 2);
        dirtThrowButton->setPosition(buttonPos);
        dirtThrowArc->setPosition(buttonPos);
        dirtThrowArc->setAngle(arc_rotate_angle);
        if ((myCurBoard == 1 && _input.getDir().x == 1) || (myCurBoard == -1 && _input.getDir().x == -1)) {
            if (_currentTutorialStage != THROW) { // don't allow returning to own board before player has thrown dirt for the first time
                ifSwitch = true;
                if (_currentTutorialStage == RETURN) { _currentTutorialStage = ALL_DIRT; } // player has returned to their board after throwing dirt
            }
        }
        if (_currentDirtAmount > 0) {
            // _dirtThrowInput.update();
            float player_x = myCurBoard == 1 ? getSize().width - _windowVec[_id - 1]->sideGap : _windowVec[_id - 1]->sideGap;
            cugl::Vec2 playerPos(player_x, _playerVec[_id - 1]->getPosition().y);
            if (!_dirtSelected) {
                if (dirtCon.didPress() && dirtThrowButton->isDown()) {
                    _dirtSelected = true;
                    _prevInputPos = worldPos;
                }
            }
            else {
                ifSwitch = false;
                if (dirtCon.didRelease()) {
                    if (_currentTutorialStage == THROW) { _currentTutorialStage = RETURN; } // player has thrown dirt at opponent
                    _dirtSelected = false;
                    Vec2 diff = worldPos - _prevInputPos;
                    if ((myCurBoard == -1 && diff.x > 0) || (myCurBoard == 1 && diff.x < 0)) {
                        diff.x = 0;
                    }
                    if (diff.length() > dirtThrowArc->getWidth() / 2) {
                        diff = diff.getNormalization() * dirtThrowArc->getWidth() / 2;
                    }
                    Vec2 destination = playerPos - diff * 7;
                    Vec2 snapped_dest = getBoardPosition(destination);
                    snapped_dest.x = clamp(round(snapped_dest.x), 0.0f, (float)_windowVec[_id - 1]->getNHorizontal()) + 0.5;
                    snapped_dest.y = clamp(round(snapped_dest.y), 0.0f, (float)_windowVec[_id - 1]->getNVertical()) + 0.5;
                    snapped_dest = getWorldPosition(snapped_dest);
                    Vec2 velocity = (snapped_dest - playerPos).getNormalization() * 8;
                    int targetId = calculateNeighborId(_id, myCurBoard, _playerVec);

                    if (_ishost) {
                        processDirtThrowRequest(netStructs.deserializeDirtRequest(*getDirtThrowRequest(targetId, playerPos, velocity, snapped_dest, _currentDirtAmount)));
                    }
//                    else {
//                        _network.sendToHost(*getDirtThrowRequest(targetId, playerPos, velocity, snapped_dest, _currentDirtAmount));
//                    }
                    dirtThrowButton->setPosition(buttonPos);
                }
                else if (dirtCon.isDown()) {
                    // cugl::Vec2 buttonPos(button_x, dirtThrowButton->getPositionY());
                    std::vector<Vec2> vertices;
                    vertices.push_back(playerPos);
                    Vec2 diff = worldPos - _prevInputPos;
                    if ((myCurBoard == -1 && diff.x > 0) || (myCurBoard == 1 && diff.x < 0)) {
                        diff.x = 0;
                    }
                    if (diff.length() > dirtThrowArc->getWidth() / 2) {
                        diff = diff.getNormalization() * dirtThrowArc->getWidth() / 2;
                    }
                    Vec2 destination = playerPos - diff * 7;
                    dirtThrowButton->setPosition(buttonPos + diff);
                    Vec2 snapped_dest = getBoardPosition(destination);
                    snapped_dest.x = clamp(round(snapped_dest.x), 0.0f, (float)_windowVec[_id - 1]->getNHorizontal()) + 0.5;
                    snapped_dest.y = clamp(round(snapped_dest.y), 0.0f, (float)_windowVec[_id - 1]->getNVertical()) + 0.5;
                    vertices.push_back(getWorldPosition(snapped_dest));
                    _dirtPath = Path2(vertices);
                }
            }
        }
        if (ifSwitch) {
            switchScene();
        }
        _lastDirtThrowButtonLocation = buttonPos;
    }

    // When a player is on their own board
    else if (myCurBoard == 0 && _gameStart) {
        // Check if player is stunned for this frame
        if (_playerVec[_id - 1]->getAnimationState() == Player::IDLE) {
            // Move the player, ignoring collisions
            int moveResult = _playerVec[_id - 1]->move(_input.getDir(), getSize(), _windowVec[_id - 1]);
            if (_currentTutorialStage < FIRST_DIRT && moveResult == 2 && !_input.getDir().equals(Vec2())) { _currentTutorialStage = FIRST_DIRT; } // player moved
            if (_currentTutorialStage >= PEEK) { // don't allow peeking until all solo mechanics introduced
                if (_numPlayers > 1 && (moveResult == -1 || moveResult == 1)) {
                    _allCurBoards[0] = moveResult;
                    if (_currentTutorialStage == PEEK) { _currentTutorialStage = THROW; } // player has moved to another player's board
                }
            }
        }
    }

    // advance bird flying frame
    _bird.advanceBirdFrame();

    // update frame count for win / lose screen
    // if a number of frames have passed,
    // we will call setRequestForMenu
    // to let app know that we should
    // switch to the main menu
    if (_gameOver) {
        _frameCountForWin = _frameCountForWin + 1;
    }

    if (_frameCountForWin > 4 * _fps && _gameOver) {
        setRequestForMenu(true);
    };
}

/**
 * Draws all this scene to the given SpriteBatch.
 *
 * The default implementation of this method simply draws the scene graph
 * to the sprite batch.  By overriding it, you can do custom drawing
 * in its place.
 *
 * @param batch     The SpriteBatch to draw with.
 */
void TutorialController::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    auto player = _playerVec[_id - 1];
    int leftId = 2;
    int rightId = 2;
    auto playerLeft = _playerVec[leftId - 1];
    auto playerRight = _playerVec[rightId - 1];
    if (_allCurBoards[_id - 1] == 0) {
        _windowVec[_id - 1]->draw(batch, getSize(), Color4(255, 255, 255, 255));
        player->draw(batch, getSize());

        // character indicators drawing start
        auto yTransLeft = playerLeft->getPosition().y;
        auto yTransRight = playerRight->getPosition().y;
        auto screenMinY = player->getPosition().y - SCENE_HEIGHT / 2.0 + 150;
        auto screenMaxY = player->getPosition().y + SCENE_HEIGHT / 2.0 - 150;

        auto leftPlayerTexture = playerLeft->getProfileTexture();
        auto rightPlayerTexture = playerRight->getProfileTexture();
        bool isAttackingOnScreen = false; // when true, ignore indicator drawing

        if (_allCurBoards[leftId - 1] == 1) {
            // left neighbor is on this player's board
            playerLeft->drawPeeking(batch, getSize(), _allCurBoards[leftId - 1], _windowVec[_id - 1]->sideGap);
            leftPlayerTexture = playerLeft->getWarnTexture();
            isAttackingOnScreen = yTransLeft < screenMaxY && yTransLeft > screenMinY;
        }
        if (_allCurBoards[rightId - 1] == -1) {
            // right neighbor is on this player's board
            playerRight->drawPeeking(batch, getSize(), _allCurBoards[rightId - 1], _windowVec[_id - 1]->sideGap);
            rightPlayerTexture = playerRight->getWarnTexture();
            isAttackingOnScreen = yTransRight < screenMaxY && yTransRight > screenMinY;
        }

        if (leftId != _id && rightId != _id && !isAttackingOnScreen) {

            Affine2 leftTrans = Affine2();
            leftTrans.translate(leftPlayerTexture->getSize() * -0.5);

            Affine2 rightTrans = Affine2();
            rightTrans.translate(rightPlayerTexture->getSize() * -0.5);

            Affine2 leftTransArrow = Affine2();
            leftTransArrow.translate(_arrowTexture->getSize() * -0.5);
            leftTransArrow.scale(Vec2(-0.75, 0.75));
            Affine2 rightTransArrow = Affine2();
            rightTransArrow.translate(_arrowTexture->getSize() * -0.5);
            rightTransArrow.scale(0.75);

            if (yTransLeft > screenMaxY) {
                yTransLeft = screenMaxY;
                leftTransArrow.rotate(M_PI / 2.0);
                leftTransArrow.translate(getSize().width - _windowVec[_id - 1]->sideGap + 50, yTransLeft + 60);
                batch->draw(_arrowTexture, Vec2(), leftTransArrow);
            }
            else if (yTransLeft < screenMinY) {
                yTransLeft = screenMinY;
                leftTransArrow.rotate(3.0 * M_PI / 2.0);
                leftTransArrow.translate(getSize().width - _windowVec[_id - 1]->sideGap + 50, yTransLeft - 60);
                batch->draw(_arrowTexture, Vec2(), leftTransArrow);
            }

            if (yTransRight > screenMaxY) {
                yTransRight = screenMaxY;
                rightTransArrow.rotate(3.0 * M_PI / 2.0);
                rightTransArrow.translate(_windowVec[_id - 1]->sideGap - 50, yTransRight + 60);
                batch->draw(_arrowTexture, Vec2(), rightTransArrow);
            }
            else if (yTransRight < screenMinY) {
                yTransRight = screenMinY;
                rightTransArrow.rotate(M_PI / 2.0);
                rightTransArrow.translate(_windowVec[_id - 1]->sideGap - 50, yTransRight - 60);
                batch->draw(_arrowTexture, Vec2(), rightTransArrow);
            }

            leftTrans.translate(getSize().width - _windowVec[_id - 1]->sideGap + 50, yTransLeft);
            batch->draw(leftPlayerTexture, Vec2(), leftTrans);

            rightTrans.translate(_windowVec[_id - 1]->sideGap - 50, yTransRight);
            batch->draw(rightPlayerTexture, Vec2(), rightTrans);
        }
        // character indicators drawing end

        _projectileVec[_id - 1]->draw(batch, getSize(), _windowVec[_id - 1]->getPaneWidth(), _windowVec[_id - 1]->getPaneHeight());
        if (_curBirdBoard == _id) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    else if (_allCurBoards[_id - 1] == -1 && leftId != _id) {
        _windowVec[leftId - 1]->draw(batch, getSize(), playerLeft->getColor());
        if (_allCurBoards[leftId - 1] == 0) {
            playerLeft->draw(batch, getSize());
        }
        player->drawPeeking(batch, getSize(), _allCurBoards[_id - 1], _windowVec[_id - 1]->sideGap);
        _projectileVec[leftId - 1]->draw(batch, getSize(), _windowVec[leftId - 1]->getPaneWidth(), _windowVec[leftId - 1]->getPaneHeight());

        vector<Vec2> potentialDirts;
        if (_dirtSelected && _dirtPath.size() != 0) {
            Vec2 dirtDest = _dirtPath.getVertices().back() - Vec2(0.5, 0.5);
            Vec2 landedDirtCoords = getBoardPosition(dirtDest);
            landedDirtCoords.y = std::clamp(static_cast<int>(landedDirtCoords.y), 0, _windowVec[leftId - 1]->getNVertical() - 1);
            landedDirtCoords.x = std::clamp(static_cast<int>(landedDirtCoords.x), 0, _windowVec[leftId - 1]->getNHorizontal() - 1);
            potentialDirts = calculateLandedDirtPositions(_windowVec[leftId - 1]->getNVertical(), _windowVec[leftId - 1]->getNHorizontal(), landedDirtCoords, _currentDirtAmount);
        }
        if (potentialDirts.size() > 0) {
            _windowVec[leftId - 1]->drawPotentialDirt(batch, getSize(), potentialDirts);
        }

        if (_curBirdBoard == leftId) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    else if (_allCurBoards[_id - 1] == 1 && rightId != _id) {
        _windowVec[rightId - 1]->draw(batch, getSize(), playerRight->getColor());
        if (_allCurBoards[rightId - 1] == 0) {
            playerRight->draw(batch, getSize());
        }
        player->drawPeeking(batch, getSize(), _allCurBoards[_id - 1], _windowVec[_id - 1]->sideGap);
        _projectileVec[rightId - 1]->draw(batch, getSize(), _windowVec[rightId - 1]->getPaneWidth(), _windowVec[rightId - 1]->getPaneHeight());

        vector<Vec2> potentialDirts;
        if (_dirtSelected && _dirtPath.size() != 0) {
            Vec2 dirtDest = _dirtPath.getVertices().back();
            Vec2 landedDirtCoords = getBoardPosition(dirtDest);
            landedDirtCoords.y = std::clamp(static_cast<int>(landedDirtCoords.y), 0, _windowVec[rightId - 1]->getNVertical() - 1);
            landedDirtCoords.x = std::clamp(static_cast<int>(landedDirtCoords.x), 0, _windowVec[rightId - 1]->getNHorizontal() - 1);
            potentialDirts = calculateLandedDirtPositions(_windowVec[rightId - 1]->getNVertical(), _windowVec[rightId - 1]->getNHorizontal(), landedDirtCoords, _currentDirtAmount);
        }
        if (potentialDirts.size() > 0) {
            _windowVec[rightId - 1]->drawPotentialDirt(batch, getSize(), potentialDirts);
        }
        if (_curBirdBoard == rightId) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    // draw tutorial instructions
    Size screenSize = getSize();
    shared_ptr<Texture> textTexture = _TutorialTexts.at(_currentTutorialStage);
    Size textBoxSize = textTexture->getSize();
    float textBoxScaleFactor = .3;
    Affine2 textBoxTrans = Affine2();
    textBoxTrans.translate(-.5 * textBoxSize.getIWidth(), 0);
    textBoxTrans.scale(textBoxScaleFactor);
    textBoxTrans.translate(player->getPosition());
    textBoxTrans.translate(0, -1.5 * _windowVec[0]->getPaneHeight());
    if (_currentTutorialStage == THROW) {
        textBoxTrans.translate(0, -.5 * _windowVec[0]->getPaneHeight());
    }
    batch->draw(textTexture, Vec2(0, 0), textBoxTrans);
  
}

void TutorialController::drawTutorialFinger(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    if (_currentTutorialStage != THROW) {
        return;
    }
    fingerT += .015;
    if (fingerT > 1) {
        fingerT = 0;
    }
    auto player = _playerVec[_id - 1];
    Affine2 dragFingerTrans = Affine2();
    dragFingerTrans.translate(-.3 * _TutorialDragFinger->getSize().getIWidth(), -.9 * _TutorialDragFinger->getSize().getIHeight());
    float fingerScaleFactor = .1;
    dragFingerTrans.scale(fingerScaleFactor);
    dragFingerTrans.translate(_lastDirtThrowButtonLocation + Vec2(0, player->getPosition().y - 350));
    Vec2 animationTarget = (player->getCoors().x == 0) ? Vec2(-_fingerTargetDelta.x, _fingerTargetDelta.y) : _fingerTargetDelta;
    Vec2 animationTranslation = animationTarget.scale(fingerT, fingerT);
    dragFingerTrans.translate(animationTranslation);
    batch->draw(_TutorialDragFinger, Vec2(), dragFingerTrans);
    CULog("%f\n", fingerT);
}
