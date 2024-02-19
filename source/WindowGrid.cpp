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
	 _nHorizontal = data->getInt("columns", 2);
	 _nVertical = data->getInt("rows", 4);

	 // calculate scale and size of window pane drawing
	 _scaleFactor = std::min(((float)size.getIWidth() / (float)_texture->getWidth() / (float)_nHorizontal), ((float)size.getIHeight() / (float)_texture->getHeight() / (float)_nVertical));
	 _windowWidth = (float)_texture->getWidth() * _scaleFactor; // final width of each window pane
	 _windowHeight = (float)_texture->getHeight() * _scaleFactor; // final height of each window pane
	 sideGap = ((float)size.getIWidth() - _windowWidth * _nHorizontal) / 2; // final gap width from side of screen to side of building

	// Initialize the dirt board
	_board = std::vector<std::vector<bool>>(_nHorizontal, std::vector<bool>(_nVertical, false));
	int n_dirt = data->getInt("number dirts", 1);

	std::mt19937 rng(std::time(nullptr)); // Initializes random number generator
	std::uniform_int_distribution<int> rowDist(0, _board.size() - 1);
	std::uniform_int_distribution<int> colDist(0, _board[0].size() - 1);
	for (int i = 0; i < n_dirt; ++i) {
		int rand_row = rowDist(rng);
		int rand_col = colDist(rng);

		// if element is already true, select different element
		if (_board[rand_row][rand_col]) {
			--i; // Decrease i to repeat this iteration
			continue;
		}
		_board[rand_row][rand_col] = true;
	}

	return true;
}

// draws an entire grid of _nHorizontal x nVertical windows as large as possible with center (horizontal) alignment
void WindowGrid::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size) {
	
	// get scale and size of window pane drawing as transform
	// scale applied to each window pane
	Affine2 trans = Affine2();
	trans.scale(_scaleFactor);
	trans.translate(sideGap, 0);

	// calculate scale and size of dirt drawing in reference to a window pane so that it is centered
	// scale applied to each dirt tile
	float dirtScaleFactor = std::min(((float)size.getIWidth() / (float)_dirt->getWidth() / (float)_nHorizontal), ((float)size.getIHeight() / (float)_dirt->getHeight() / (float)_nVertical));
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
			// draw window panes and dirt
			batch->draw(_texture, Vec2(), trans);
			if (_board[x][y]) {
				batch->draw(_dirt, Vec2(), dirt_trans);
				CULog("dirt added to coors: (%d, %d)", x, y);
			}

			// update vertical translation
			trans.translate(0, _windowHeight);
			dirt_trans.translate(0, _windowHeight);
		}

		// update horizontal translation and reset vertical translation
		trans.translate(_windowWidth, -_windowHeight * _nVertical);
		dirt_trans.translate(_windowWidth, -_windowHeight * _nVertical);
	}
}

#endif
