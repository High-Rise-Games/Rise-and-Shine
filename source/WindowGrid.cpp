#ifndef __WINDOW_GRID_C__
#define __WINDOW_GRID_C__

#include "WindowGrid.h"

using namespace cugl;

WindowGrid::WindowGrid() {
    // such constructor
    // much wow
    
    // incredible work here -cathryn
}

bool WindowGrid::init(int nHorizontal, int nVertical, cugl::Size size) {
    _nHorizontal = nHorizontal;
    _nVertical = nVertical;

    return true;
}

bool WindowGrid::init(std::shared_ptr<cugl::JsonValue> data, cugl::Size size) {
     _nHorizontal = data->getInt("width", 2);
     _nVertical = data->getInt("height", 4);
 
	 // compute mapping from Tiled ids to _textures index
	 int i = 0;
	 for (int texture_id : _texture_ids) {
		 _texture_indices[texture_id] = i;
		 i++;
	 }

	 _window_map.clear();
	 _left_blocked_map.clear();
	 _down_blocked_map.clear();
	 _fully_blocked_map.clear();
	 std::vector<std::shared_ptr<cugl::JsonValue>> layers = data->get("layers")->children();
	 for (std::shared_ptr<cugl::JsonValue> l : layers) {
		 if (l->getString("name") == "Building") {
			 // TODO: set building constants?
			 std::shared_ptr<cugl::JsonValue> buildingObject = l->get("objects")->children().at(0);
			 int buildingTextureIndex = buildingObject->get("gid")->asInt();
			 setBuildingTexture(_textures[_texture_indices[buildingTextureIndex]]);
			 float buildingTextureX = buildingObject->get("x")->asFloat();
			 float buildingTextureY = buildingObject->get("y")->asFloat();
			 _buildingTexturePosition = Vec2(buildingTextureX, buildingTextureY);
		 }
		 else if (l->getString("name") == "Windows") {
             std::vector<int> temp = l->get("data")->asIntArray();
             for (int i = _nHorizontal*_nVertical; i>0; i=i-_nHorizontal) {
                 for (int ii=_nHorizontal; ii>0; ii--) {
					 _window_map.push_back(temp.at(i-ii));
                 }
             }
		 }
		 else if (l->getString("name") == "Pipes Left") {
			 std::vector<int> temp = l->get("data")->asIntArray();
			 for (int i = _nHorizontal * _nVertical; i > 0; i = i - _nHorizontal) {
				 for (int ii = _nHorizontal; ii > 0; ii--) {
					 _left_blocked_map.push_back(temp.at(i - ii));
				 }
			 }
		 }
		 else if (l->getString("name") == "Pipes Down") {
			 std::vector<int> temp = l->get("data")->asIntArray();
			 for (int i = _nHorizontal * _nVertical; i > 0; i = i - _nHorizontal) {
				 for (int ii = _nHorizontal; ii > 0; ii--) {
					 _down_blocked_map.push_back(temp.at(i - ii));
				 }
			 }
		 }
		 else if (l->getString("name") == "Blocked Tiles") {
			 std::vector<int> temp = l->get("data")->asIntArray();
			 for (int i = _nHorizontal * _nVertical; i > 0; i = i - _nHorizontal) {
				 for (int ii = _nHorizontal; ii > 0; ii--) {
					 _fully_blocked_map.push_back(temp.at(i - ii));
				 }
			 }
		 }
	 }

     int tileHeight = data->getInt("tileheight", 2);
     int tileWidth = data->getInt("tilewidth", 2);

	 // calculate scale and size of window grid
	 _scaleFactor = std::min(((float)size.getIWidth() / (float)tileWidth / (float)_nHorizontal), ((float)size.getIHeight() / (float)tileHeight / std::min((float)MAX_HEIGHT, (float)_nVertical)));
	 _windowWidth = (float)tileWidth * _scaleFactor; // final width of each window grid
	 _windowHeight = (float)tileHeight * _scaleFactor; // final height of each window grid
	 sideGap = ((float)size.getIWidth() - _windowWidth * _nHorizontal) / 2; // final gap width from side of screen to side of building
	 _buildingTexturePosition.y = (_buildingTexturePosition.y - (tileHeight * _nVertical)) * -1;

    // Initialize the dirt board
    _boardFilth = std::vector<std::vector<std::shared_ptr<StaticFilth>>>(_nVertical, std::vector<std::shared_ptr<StaticFilth>>(_nHorizontal, nullptr));
    
    for (const std::shared_ptr<JsonValue>& property : data->get("properties")->children()) {
        if (property->getString("name") == "number dirts") {
            _initDirtNum = property->getInt("value", 1);
        }
    }
    
    for (const std::shared_ptr<JsonValue>& object : data->get("layers")->children()) {
        if (object->getString("name") == "Building") {
            const std::shared_ptr<JsonValue>& object1 = object->get("objects")->get(0);
            _buildingHeight = (object1->getInt("height")) / (size.height);
            _buildingWidth = ((object1->getInt("width"))/ size.width);
            
        }
    }

    return true;
}

cugl::Vec2 WindowGrid::getGridIndices(cugl::Vec2 location, cugl::Size size) {

    int x_coor = floor((location.x - sideGap) / _windowWidth);
    int y_coor = floor(location.y / _windowHeight);
    return Vec2(x_coor, y_coor);
    //return Vec2(-1, -1);
}

bool WindowGrid::getCanMoveBetween(int x_origin, int y_origin, int x_dest, int y_dest) {
	int originMapIndex = x_origin + getNHorizontal() * y_origin;
	int destMapIndex   = x_dest   + getNHorizontal() * y_dest;
	if (x_dest < 0 || y_dest < 0 || x_dest >= getNHorizontal() || y_dest >= getNVertical()) { // invalid destination
		return false;
	}
	bool impassableDest = _fully_blocked_map.at(destMapIndex) != 0; // any tile present in this layer blocks passage completely
	if (impassableDest) { // dest is not a tile the player can ever access
		return false;
	}
	// check relevant directional blockages on both tiles involved, allow if neither tile blocks
	if (y_origin == y_dest) { // horizontal move
		if (x_dest == x_origin + 1) { // right
			return _left_blocked_map.at(destMapIndex) == 0;
		}
		else if (x_dest == x_origin - 1) { // left
			return _left_blocked_map.at(originMapIndex) == 0;
		}
	}
	else if (x_origin == x_dest) { // vertical move
		if (y_dest == y_origin + 1) { // up
			return _down_blocked_map.at(destMapIndex) == 0;
		}
		else if (y_dest == y_origin - 1) { // down
			return _down_blocked_map.at(originMapIndex) == 0;
		}
	}
	return false; // don't allow unanticipated movement modes
}

bool WindowGrid::getCanBeDirtied(int x_index, int y_index) {
	int tileMapIndex = x_index + getNHorizontal() * y_index;
	return _fully_blocked_map.at(tileMapIndex) == 0; // check that tile is not inaccessible
}

void WindowGrid::generateInitialBoard(int dirtNumber) {
    std::mt19937 rng(std::time(nullptr)); // Initializes random number generator
    std::uniform_int_distribution<int> rowDist(0, _boardFilth.size() - 1);
    std::uniform_int_distribution<int> colDist(0, _boardFilth[0].size() - 1);
    for (int i = 0; i < dirtNumber; ++i) {
        int rand_row = rowDist(rng);
        int rand_col = colDist(rng);

        // if dirt already present or tile not accessible, select different element
        if (_boardFilth[rand_row][rand_col] || !getCanBeDirtied(rand_col, rand_row)) {
            --i; // Decrease i to repeat this iteration
            continue;
        }
        std::shared_ptr<StaticFilth> dirt = std::make_shared<StaticFilth>(Vec2(rand_row, rand_col));
        dirt->setStaticTexture(_dirt);
        _boardFilth[rand_row][rand_col] = dirt;
    }
}

void WindowGrid::clearBoard() {
    for (int x = 0; x < _nHorizontal; x++) {
        for (int y = 0; y < _nVertical; y++) {
            _boardFilth[y][x].reset();
            _boardFilth[y][x] = nullptr;
        }
    }
}

bool WindowGrid::addDirt(const int row, const int col) {
    bool dirtExisted = _boardFilth[row][col] != nullptr;
    bool isTileDirtiable = getCanBeDirtied(col, row);
    if (!dirtExisted && isTileDirtiable) {
        std::shared_ptr<StaticFilth> filth = std::make_shared<StaticFilth>(cugl::Vec2(row, col));
        filth->setStaticTexture(_dirt);
        _boardFilth[row][col] = filth;
    }
    return !dirtExisted && isTileDirtiable;
}

/**
* Clears grid-related texture variables for a fresh start every time we reuse the window grid for a new level
*/
void WindowGrid::clearWindowTextures() {
	_textures.clear();
	_texture_ids.clear();
	_texture_indices.clear();
}

// draws an entire grid of _nHorizontal x nVertical windows as large as possible with center (horizontal) alignment
void WindowGrid::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, Color4 tint) {
	// draw building background
	Affine2 building_trans = Affine2();
    //building_trans.scale(getPaneWidth() * _nHorizontal / _buildingTexture->getWidth(), getPaneHeight() * _nVertical / _buildingTexture->getHeight());
	building_trans.translate(_buildingTexturePosition);
	building_trans.scale(_scaleFactor);
	building_trans.translate(sideGap, 0);
	
	// batch->setColor(tint);
	batch->draw(_buildingTexture, tint, Vec2(), building_trans);
	
	// calculate scale and size of dirt drawing in reference to a window pane so that it is centered
	// scale applied to each dirt tile
	// float dirtScaleFactor = std::min(((float)size.width / (float)_dirt->getWidth() / (float)_nHorizontal), ((float)size.height / (float)_dirt->getHeight() / (float)_nVertical));
	
	float dirtScaleFactor = std::min(_windowWidth / _dirt->getWidth(), _windowHeight / _dirt->getHeight()) * 0.75;
	float dirtWidth = (float)_dirt->getWidth() * dirtScaleFactor;
	float dirtHeight = (float)_dirt->getHeight() * dirtScaleFactor;

    float dirt_horizontal_trans = (_windowWidth - dirtWidth) / 2;
    float dirt_vertical_trans = (_windowHeight - dirtHeight) / 2;

    Affine2 dirt_trans = Affine2();
    dirt_trans.scale(dirtScaleFactor);
    dirt_trans.translate(sideGap + dirt_horizontal_trans, dirt_vertical_trans);

	// loop over all grid points and draw window panes and dirt
	for (int x = 0; x < _nHorizontal; x++) {
		for (int y = 0; y < _nVertical; y++) {
			int mapIdx = y * _nHorizontal + x;
			std::shared_ptr<cugl::Texture> window_texture  = _window_map[mapIdx]        == 0 ? NULL : _textures[_texture_indices[_window_map       [mapIdx]]];
			std::shared_ptr<cugl::Texture> left_texture    = _left_blocked_map[mapIdx]  == 0 ? NULL : _textures[_texture_indices[_left_blocked_map [mapIdx]]];
			std::shared_ptr<cugl::Texture> down_texture    = _down_blocked_map[mapIdx]  == 0 ? NULL : _textures[_texture_indices[_down_blocked_map [mapIdx]]];
			std::shared_ptr<cugl::Texture> blocked_texture = _fully_blocked_map[mapIdx] == 0 ? NULL : _textures[_texture_indices[_fully_blocked_map[mapIdx]]];

			// get scale and size of window pane drawing as transform
			Affine2 trans = Affine2();
			Affine2 blocked_trans = Affine2();
			float paneScaleFactor = std::min(_windowHeight / (float)window_texture->getHeight(), _windowWidth / (float)window_texture->getWidth()) * 0.9;
			float blockedScaleFactor = blocked_texture != NULL ? std::min(_windowHeight / (float)blocked_texture->getHeight(), _windowWidth / (float)blocked_texture->getWidth()) * 0.9 : 1;
			float paneWidth = (float)window_texture->getWidth() * paneScaleFactor;
			float paneHeight = (float)window_texture->getHeight() * paneScaleFactor;
			float pipeDownOffset = paneHeight * .2; // push pipes into gaps between windows
			float pipeLeftOffset = paneWidth  * .2; // push pipes into gaps between windows

            float pane_horizontal_trans = (_windowWidth - paneWidth) / 2;
            float pane_vertical_trans = (_windowHeight - paneHeight) / 2;
            trans.scale(paneScaleFactor);
			blocked_trans.scale(blockedScaleFactor);
            trans.translate(sideGap + (_windowWidth * x) + pane_horizontal_trans, (_windowHeight * y) + pane_vertical_trans);
			blocked_trans.translate(sideGap + (_windowWidth * x) + pane_horizontal_trans, (_windowHeight * y) + pane_vertical_trans);

			Affine2 leftTrans = Affine2(trans).translate(-pipeLeftOffset, 0);
			Affine2 downTrans = Affine2(trans).translate(0, -pipeDownOffset);

			// draw window panes and dirt
			if (window_texture  != NULL) { batch->draw(window_texture,  Vec2(), trans); }
			if (left_texture    != NULL) { batch->draw(left_texture,    Vec2(), leftTrans); }
			if (down_texture    != NULL) { batch->draw(down_texture,    Vec2(), downTrans); }
			if (blocked_texture != NULL) { batch->draw(blocked_texture, Vec2(), blocked_trans); }
			
			if (_boardFilth[y][x] != nullptr) {
				_boardFilth[y][x]->drawStatic(batch, size, dirt_trans);
			}

            // update vertical translation for dirt
            dirt_trans.translate(0, _windowHeight);
        }

        // update horizontal translation and reset vertical translation for dirt
        dirt_trans.translate(_windowWidth, -_windowHeight * _nVertical);
    }
}

void WindowGrid::drawPotentialDirt(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, std::vector<cugl::Vec2> potentialFilth) {
    float dirtScaleFactor = std::min(_windowWidth / _fadedDirtTexture->getWidth(), _windowHeight / _fadedDirtTexture->getHeight()) * 0.75;
    float dirtWidth = (float)_fadedDirtTexture->getWidth() * dirtScaleFactor;
    float dirtHeight = (float)_fadedDirtTexture->getHeight() * dirtScaleFactor;

    float dirt_horizontal_trans = (_windowWidth - dirtWidth) / 2;
    float dirt_vertical_trans = (_windowHeight - dirtHeight) / 2;

    for (auto coords : potentialFilth) {
        Affine2 dirt_trans = Affine2();
        dirt_trans.scale(dirtScaleFactor);
        dirt_trans.translate(sideGap + (_windowWidth * coords.x) + dirt_horizontal_trans, (_windowHeight * coords.y) + dirt_vertical_trans);
        batch->draw(_fadedDirtTexture, Vec2(), dirt_trans);
    }
}

#endif
