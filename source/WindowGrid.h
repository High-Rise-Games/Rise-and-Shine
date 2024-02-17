/*
Class representing the drawable grid of windows in the background.
Essentially, it automatically scales and tiles the given window texture to fit the screen
with the desired number of rows and columns.
*/

#ifndef __WINDOW_GRID_H__
#define __WINDOW_GRID_H__

#include <cugl/cugl.h>

class WindowGrid {
private:
	//int windowWidth;
	//int windowHeight;
	int _nHorizontal; // number of columns
	int _nVertical;   // number of rows

	std::shared_ptr<cugl::Texture> _texture;


public:
	// accessors 
	//void setWindowWidth(int width)        { windowWidth = width;       };
	//int  getWindowWidth()                 { return windowWidth;        };
	//void setWindowHeight(int height)      { windowHeight = height;     };
	//int  getWindowHeight()                { return windowHeight;       };
	void setNHorizontal(int n)            { _nHorizontal = n;           };
	int  getNHorizontal()                 { return _nHorizontal;        };
	void setNVertical(int n)              { _nVertical = n;             };
	int  getNVertical()                   { return _nVertical;          };
    float sideGap;

	WindowGrid(); // constructor

	bool init(int nHorizontal, int nVertical); // sets number of windows in grid

	void setTexture(const std::shared_ptr<cugl::Texture>& value) { _texture = value; }; // sets window pane texture

	const std::shared_ptr<cugl::Texture>& getTexture() const { // gets window pane texture
		return _texture;
	}

	// draws entire grid of window panes to fit in "size"
	void draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size);
};

#endif
