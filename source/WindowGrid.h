
#include <cugl/cugl.h>

class WindowGrid {
private:
	int width;
	int height;
	int horizontalSpacing;
	int verticalSpacing;
public:
	// accessors 
	void setWidth(int width)                { width = width;               };
	int  getWidth()                         { return width;                };
	void setHeight(int height)              { height = height;             };
	int  getHeight()                        { return height;               };
	void setHorizontalSpacing(int spacing)  { horizontalSpacing = spacing; };
	int  getHorizontalSpacing()             { return horizontalSpacing;    };
	void setVerticalSpacing(int spacing)    { verticalSpacing = spacing;   };
	int  getVerticalSpacing()               { return verticalSpacing;      };
};