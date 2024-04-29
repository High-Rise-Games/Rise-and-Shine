/*
Class representing the drawable grid of windows in the background.
Essentially, it automatically scales and tiles the given window texture to fit the screen
with the desired number of rows and columns.
*/

#ifndef __WINDOW_GRID_H__
#define __WINDOW_GRID_H__

#include <cugl/cugl.h>
#include "StaticFilth.h"

class WindowGrid {
private:
    int _nHorizontal; // number of columns
    int _nVertical;   // number of rows
    /** initial dirt number */
    int _initDirtNum;
    float _scaleFactor;
    float _windowHeight;
    float _windowWidth;
    float _buildingWidth;
    float _buildingHeight;
	cugl::Vec2 _buildingTexturePosition;
	/** Map of window tile layer, for drawing only */
	std::vector<int> _window_map;
	/** Map of left-facing blockages, prevents movement through that side of a tile */
	std::vector<int> _left_blocked_map;
	/** Map of bottom-facing blockages, prevents movement through that side of a tile */
	std::vector<int> _down_blocked_map;
	/** Map of completely blocked tiles, prevents movement to that tile at all */
	std::vector<int> _fully_blocked_map;
	/** Tile ids that players cannot move to at all */
	//std::set<int> _impassableTiles    = { 3 };
	/** Tile ids that players cannot move through top side of */
	//std::set<int> _topBlockedTiles    = { 4, 8, 10, 13 };
	/** Tile ids that players cannot move through bottom side of */
	//std::set<int> _bottomBlockedTiles = { 6, 8, 11, 12 };
	/** Tile ids that players cannot move through right side of */
	//std::set<int> _rightBlockedTiles  = { 5, 9, 10, 11 };
	/** Tile ids that players cannot move through left side of */
	//std::set<int> _leftBlockedTiles   = { 7, 9, 12, 13 };
	/** Tile ids that cannot hold dirt */
	//std::set<int> _noDirtTiles        = { 3 };

	/** Filth placement state */
	std::vector<std::vector<std::shared_ptr<StaticFilth>>> _boardFilth;

	/** Building background texture */
	std::shared_ptr<cugl::Texture> _buildingTexture;
	/** Window texture images */
	std::vector<std::shared_ptr<cugl::Texture>> _textures;
	/** Texture id mapping from _textures index to Tiled id */
	std::vector<int> _texture_ids;
	/** mapping from Tiled id to _textures index */
	std::map<int, int> _texture_indices;

	/** Dirt texture image */
	std::shared_ptr<cugl::Texture> _dirt;
	/** Faded dirt texture image for potential dirts when aiming */
	std::shared_ptr<cugl::Texture> _fadedDirtTexture;


public:
    const int MAX_HEIGHT = 15; // TODO change to 4 once scrolling is complete
    // accessors
//    void setWindowWidth(int width)        { _windowWidth = width;       };
//    int  getWindowWidth()                 { return _windowWidth;        };
//    void setWindowHeight(int height)      { _windowHeight = height;     };
//    int  getWindowHeight()                { return _windowHeight;       };
    void setNHorizontal(int n)            { _nHorizontal = n;           };
    int  getNHorizontal()                 { return _nHorizontal;        };
    void setNVertical(int n)              { _nVertical = n;             };
    int  getNVertical()                   { return _nVertical;          };
    void  setInitDirtNum(int dirtN)       { _initDirtNum = dirtN;       };
    int  getInitDirtNum()                 { return _initDirtNum;        };
    float sideGap;

    WindowGrid(); // constructor

    /** sets number of windows in grid */
    bool init(int nHorizontal, int nVertical, cugl::Size size);

    /** initializes window based on json data
        includes number rows, columns, and dirts to be displayed */
    bool init(std::shared_ptr<cugl::JsonValue> data, cugl::Size size);

    /** appends a texture to the texture vector */
    void addTexture(const std::shared_ptr<cugl::Texture>& value) { _textures.push_back(value); }

    /** sets window pane texture */
    void setTexture(const std::shared_ptr<cugl::Texture>& value, int idx) { _textures[idx] = value; }

    /** sets dirt texture */
    void setDirtTexture(const std::shared_ptr<cugl::Texture>& value) { _dirt = value; }

    /** sets the faded dirt texture */
    void setFadedDirtTexture(const std::shared_ptr<cugl::Texture>& value) { _fadedDirtTexture = value; }

	/** sets building texture */
	void setBuildingTexture(const std::shared_ptr<cugl::Texture>& value) { _buildingTexture = value; }

	/** sets the texture id mapping */
	void setTextureIds(std::vector<int> texture_ids) { _texture_ids = texture_ids; }
	
	/** gets window pane texture */
	const std::shared_ptr<cugl::Texture>& getTexture(int idx) const {
		return _textures[idx];
	}

    /** Returns the window height */
    const float getPaneHeight() const { return _windowHeight; }

    /** Returns the window width */
    const float getPaneWidth() const { return _windowWidth; }
    
    /**
     * Get window state at row and col
     */
    bool getWindowState(const int row, const int col) {
        return _boardFilth[row][col] != nullptr ;
    }
	
	/** Returns total amount of dirt on board */
	int getTotalDirt() {
		int count = 0;
		for (int i = 0; i < _nVertical; i++) {
			for (int j = 0; j < _nHorizontal; j++) {
				count += getWindowState(i, j);
			}
		}
		return count;
	}

	/**
	 * Get discrete indices of the window tile that a given vector is on
	 */
	cugl::Vec2 getGridIndices(cugl::Vec2 location, cugl::Size size);

    /**
     * Returns whether it is possible to move from one window grid location to another
     */
    bool getCanMoveBetween(int x_origin, int y_origin, int x_dest, int y_dest);

    /**
     * Returns whether it is possible to move from one window grid location to another
     */
    bool getCanBeDirtied(int x_index, int y_index);
    
    /**
     * Initializes the board by creating dirt
     *  @param int number of dirt to generate
     */
    void generateInitialBoard(int dirtNumber);
    
    /**
     * Remove all dirt on Board
     */
    void clearBoard();
    
    /**
     * Add dirt to board at specified location.
     * Returns true if the dirt was successfully added, and false if there is already dirt at the location or the location is inaccessible.
     */
    bool addDirt(const int row, const int col);

    /**
     * Check dirt exists from board at specified location
     * Returns true if the dirt is present.
     */
    bool hasDirt(const int row, const int col) {
//        CULog("size: %d, %d", _board.size(), board[0].size());
        bool dirtExisted = _boardFilth[row][col] != nullptr;
        return dirtExisted;
    }
    
    /**
     * Remove dirt from board at specified location
     * Returns true if the dirt was successfully removed, and false if there is no dirt to remove.
     */
    bool removeDirt(const int row, const int col) {
//        CULog("size: %d, %d", _board.size(), board[0].size());
        bool dirtExisted = hasDirt(row, col);
        if (dirtExisted) {
            _boardFilth[row][col].reset();
            _boardFilth[row][col] = nullptr;
        }
		return dirtExisted;
	}

	/**
	 * Clears grid-related texture variables for a fresh start every time we reuse the window grid for a new level
	 */
	void clearWindowTextures();

    /** draws entire grid of window panes to fit in "size" */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size);

    /** draws potential dirts when aiming */
    void drawPotentialDirt(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, std::vector<cugl::Vec2> potentialFilth);
};

#endif
