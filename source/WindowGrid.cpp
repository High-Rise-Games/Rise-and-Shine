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

	 std::vector<std::shared_ptr<cugl::JsonValue>> layers = data->get("layers")->children();
	 for (std::shared_ptr<cugl::JsonValue> l : layers) {
		 if (l->getString("name") == "Building") {
			 // TODO: set building constants?
		 }
		 else if (l->getString("name") == "Windows") {
             std::vector<int> temp = l->get("data")->asIntArray();
             for (int i = _nHorizontal*_nVertical; i>0; i=i-_nHorizontal) {
                 for (int ii=_nHorizontal; ii>0; ii--) {
                     _map.push_back(temp.at(i-ii)-1);
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

	// Initialize the dirt board
	_boardFilth = std::vector<std::vector<std::shared_ptr<StaticFilth>>>(_nVertical, std::vector<std::shared_ptr<StaticFilth>>(_nHorizontal, nullptr));
    
    for (const std::shared_ptr<JsonValue>& property : data->get("properties")->children()) {
        if (property->getString("name") == "number dirts") {
            _initDirtNum = property->getInt("value", 1);
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
	bool impassableDest = _impassableTiles.count(_map.at(destMapIndex));
	if (impassableDest) { // dest is not a tile the player can ever access
		return false;
	}
	// check relevant directional blockages on both tiles involved, allow if neither tile blocks
	if (y_origin == y_dest) { // horizontal move
		if (x_dest == x_origin + 1) { // right
			return ! ( _rightBlockedTiles.count(_map.at(originMapIndex))  ||   _leftBlockedTiles.count(_map.at(destMapIndex)) );
		}
		else if (x_dest == x_origin - 1) { // left
			return ! ( _leftBlockedTiles.count(_map.at(originMapIndex))   ||   _rightBlockedTiles.count(_map.at(destMapIndex)) );
		}
	}
	else if (x_origin == x_dest) { // vertical move
		if (y_dest == y_origin + 1) { // up
			return ! ( _topBlockedTiles.count(_map.at(originMapIndex))    ||   _bottomBlockedTiles.count(_map.at(destMapIndex)) );
		}
		else if (y_dest == y_origin - 1) { // down
			return ! ( _bottomBlockedTiles.count(_map.at(originMapIndex)) ||   _topBlockedTiles.count(_map.at(destMapIndex)) );
		}
	}
	return false; // don't allow unanticipated movement modes
}

bool WindowGrid::getCanBeDirtied(int x_index, int y_index) {
	int tileMapIndex = x_index + getNHorizontal() * y_index;
	return !( _noDirtTiles.count(_map.at(tileMapIndex)) ); // check if tile is on list of non-dirtiable tiles
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

// draws an entire grid of _nHorizontal x nVertical windows as large as possible with center (horizontal) alignment
void WindowGrid::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size) {
	// draw building background
	Affine2 building_trans = Affine2();
	building_trans.scale(std::max(_windowWidth * _nHorizontal / _buildingTexture->getWidth(), _windowHeight * _nVertical / _buildingTexture->getHeight()));
	building_trans.translate(sideGap, 0);
	batch->draw(_buildingTexture, Vec2(), building_trans);
	
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
			std::shared_ptr<cugl::Texture> texture = _textures[_map[mapIdx] - 1];

			// get scale and size of window pane drawing as transform
			Affine2 trans = Affine2();
			float paneScaleFactor = std::min(_windowHeight / (float)texture->getHeight(), _windowWidth / (float)texture->getWidth()) * 0.9;
			float paneWidth = (float)texture->getWidth() * paneScaleFactor;
			float paneHeight = (float)texture->getHeight() * paneScaleFactor;

			float pane_horizontal_trans = (_windowWidth - paneWidth) / 2;
			float pane_vertical_trans = (_windowHeight - paneHeight) / 2;
			trans.scale(paneScaleFactor);
			trans.translate(sideGap + (_windowWidth * x) + pane_horizontal_trans, (_windowHeight * y) + pane_vertical_trans);

			// draw window panes and dirt
			batch->draw(texture, Vec2(), trans);
			if (_boardFilth[y][x] != nullptr) {
				_boardFilth[y][x]->drawStatic(batch, size, dirt_trans);
//				CULog("dirt added to coors: (%d, %d)", x, y);
			}

			// update vertical translation for dirt
			dirt_trans.translate(0, _windowHeight);
		}

		// update horizontal translation and reset vertical translation for dirt
		dirt_trans.translate(_windowWidth, -_windowHeight * _nVertical);
	}
}

#endif
