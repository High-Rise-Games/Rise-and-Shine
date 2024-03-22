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
			 _map = l->get("data")->asIntArray();
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
    _initDirtNum = data->getInt("number dirts", 1);

	return true;
}

void WindowGrid::generateInitialBoard(int dirtNumber) {
    std::mt19937 rng(std::time(nullptr)); // Initializes random number generator
    std::uniform_int_distribution<int> rowDist(0, _boardFilth.size() - 1);
    std::uniform_int_distribution<int> colDist(0, _boardFilth[0].size() - 1);
    for (int i = 0; i < dirtNumber; ++i) {
        int rand_row = rowDist(rng);
        int rand_col = colDist(rng);

        // if element is already true, select different element
        if (_boardFilth[rand_row][rand_col]) {
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
