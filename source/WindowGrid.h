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
	int _nHorizontal; // number of columns
	int _nVertical;   // number of rows
	float _scaleFactor;
	float _windowHeight;
	float _windowWidth;
	/** Dirt placement state */
	std::vector<std::vector<bool>> _board;

	/** Window texture image */
	std::shared_ptr<cugl::Texture> _texture;
	/** Dirt texture image */
	std::shared_ptr<cugl::Texture> _dirt;


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

	/** sets number of windows in grid */
	bool init(int nHorizontal, int nVertical, cugl::Size size);

	/** initializes window based on json data
		includes number rows, columns, and dirts to be displayed */
	bool init(std::shared_ptr<cugl::JsonValue> data, cugl::Size size);

	/** sets window pane texture */
	void setTexture(const std::shared_ptr<cugl::Texture>& value) { _texture = value; }

	/** sets dirt texture */
	void setDirtTexture(const std::shared_ptr<cugl::Texture>& value) { _dirt = value; }
	
	/** gets window pane texture */
	const std::shared_ptr<cugl::Texture>& getTexture() const {
		return _texture;
	}

	/** Returns the window height */
	const float getPaneHeight() const { return _windowHeight; }

	/** Returns the window width */
	const float getPaneWidth() const { return _windowWidth; }

	/** 
	 * Add dirt to board at specified location.
	 * Returns true if the dirt was successfully added, and false if there is already dirt at the location.
	 */
	bool addDirt(const int row, const int col) { 
		bool dirtExisted = _board[row][col];
		_board[row][col] = true; 
		return !dirtExisted;
	}

	/** 
	 * Remove dirt from board at specified location 
	 * Returns true if the dirt was successfully removed, and false if there is no dirt to remove.
	 */
	bool removeDirt(const int row, const int col) { 
		bool dirtExisted = _board[row][col];
		_board[row][col] = false; 
		return dirtExisted;
	}

	/** draws entire grid of window panes to fit in "size" */
	void draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size);
};

#endif
