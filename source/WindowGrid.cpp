#ifndef __WINDOW_GRID_C__
#define __WINDOW_GRID_C__

#include "WindowGrid.h"

using namespace cugl;

WindowGrid::WindowGrid() {
	// such constructor
	// much wow
}

bool WindowGrid::init(int nHorizontal, int nVertical) {
	_nHorizontal = nHorizontal;
	_nVertical = nVertical;
	return true;
}

// draws an entire grid of _nHorizontal x nVertical windows as large as possible with center (horizontal) alignment
void WindowGrid::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size) {
	float scaleFactor = std::min( ((float) size.getIWidth() / (float) _texture->getWidth() / (float) _nHorizontal), ((float) size.getIHeight() / (float) _texture->getHeight() / (float) _nVertical) ); // scale applied to each window pane
	float windowWidth = (float)_texture->getWidth() * scaleFactor; // final width of each window pane
	float windowHeight = (float)_texture->getHeight() * scaleFactor; // final height of each window pane
	float sideGap = ( (float)size.getIWidth() - windowWidth * _nHorizontal ) / 2; // final gap width from side of screen to side of building
	
	// loop over all grid points and draw window panes
	for (int x = 0; x < _nHorizontal; x++) {
		Affine2 trans = Affine2();
		trans.scale(scaleFactor);
		trans.translate(windowWidth * x + sideGap, 0);
		for (int y = 0; y < _nVertical; y++) {
			batch->draw(_texture, Vec2(), trans);
			trans.translate(0, windowHeight);
		}
	}
}

#endif
