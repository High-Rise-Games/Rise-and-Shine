//
//  CUSpriteNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a straight-forward filmstrip API, similar to what we
//  use in the intro class.  Note that this class extends PolygonNode, as it
//  simply treats the node texture as a sprite sheet. Therefore, it is possible
//  to animate the filmstrip over polygons.  However, this can have undesirable
//  effects if the polygon coordinates extend beyond a single animation frame.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 12/1/16
//
#include <cugl/scene2/graph/CUSpriteNode.h>


using namespace cugl::scene2;


#pragma mark -
#pragma mark Constructors

/**
 * Constructs a ActionNode with no filmstrip
 *
 * You must initialize this object before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
SpriteNode::SpriteNode() :
_cols(0),
_size(0),
_frame(0),
_bounds(Rect::ZERO) {
    _classname = "SpriteNode";
}

/**
 * Initializes the film strip with the given texture.
 *
 * This initializer assumes that the filmstrip is rectangular, and that
 * there are no unused frames.
 *
 * The size of the node is equal to the size of a single frame in the
 * filmstrip. To resize the node, scale it up or down.  Do NOT change the
 * polygon, as that will interfere with the animation.
 *
 * @param texture   The texture image to use
 * @param rows      The number of rows in the filmstrip
 * @param cols      The number of columns in the filmstrip
 *
 * @return  true if the filmstrip is initialized properly, false otherwise.
 */
bool SpriteNode::initWithSheet(const std::shared_ptr<Texture>& texture,
                               int rows, int cols, int size) {
    CUAssertLog(size <= rows*cols, "Invalid strip size for %dx%d",rows,cols);
    
    this->_cols = cols;
    this->_size = size;
    _bounds.size = texture->getSize();
    _bounds.size.width /= cols;
    _bounds.size.height /= rows;
    return this->initWithTexturePoly(texture, _bounds);
}

/**
 * Initializes the film strip as a copy of the given one.
 *
 * @param sprite   The sprite node to copy
 *
 * @return  true if the filmstrip is initialized properly, false otherwise.
 */
bool SpriteNode::initWithSprite(const std::shared_ptr<SpriteNode>& sprite) {
    this->_cols = sprite->_cols;
    this->_size = sprite->_size;
    _bounds  = sprite->_bounds;
    return this->initWithTexturePoly(sprite->_texture, _bounds);
}

/**
 * Initializes a node with the given JSON specificaton.
 *
 * This initializer is designed to receive the "data" object from the
 * JSON passed to {@link SceneLoader}.  This JSON format supports all
 * of the attribute values of its parent class.  In addition, it supports
 * the following additional attributes:
 *
 *      "span":     The number of frames in the filmstrip
 *      "cols":     An int specifying the number of columns
 *      "frame":    The initial starting frame.
 *
 * All attributes are optional.  However, if nothing is specified, it
 * assumes that this is a degenerate filmstrip with just one frame.  If
 * only span is specified, it assumes that it is just one row.
 *
 * @param loader    The scene loader passing this JSON file
 * @param data      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool SpriteNode::initWithData(const Scene2Loader* loader, const std::shared_ptr<JsonValue>& data) {
    if (!data) {
        return TexturedNode::init();
    } else if (!TexturedNode::initWithData(loader, data)) {
        return false;
    }
    
    if (data->has("span")) {
        _size = data->getInt("span",1);
        _cols = data->getInt("cols",_size);
    } else {
        _cols = data->getInt("cols",1);
        _size = data->getInt("span",_cols);
    }
    
    int rows = _size/_cols + (_size % _cols == 0 ? 0 : 1);
    _frame = data->getInt("frame",0);
    
    // Resize the texture
    _bounds.size = _texture->getSize();
    _bounds.size.width  /= _cols;
    _bounds.size.height /= rows;
    _bounds.origin.x = (_frame % _cols)*_bounds.size.width;
    _bounds.origin.y = _texture->getSize().height - (1+_frame/_cols)*_bounds.size.height;

    // And position it correctly
    Vec2 coord = getPosition();
    setPolygon(_bounds);
    setPosition(coord);
    return true;
}

/**
 * Performs a shallow copy of this Node into dst.
 *
 * No children from this node are copied, and no children of dst are
 * modified. In addition, the parents of both Nodes are unchanged. However,
 * all other attributes of this node are copied.
 *
 * @param dst   The Node to copy into
 *
 * @return A reference to dst for chaining.
 */
std::shared_ptr<SceneNode> SpriteNode::copy(const std::shared_ptr<SceneNode>& dst) const {
    TexturedNode::copy(dst);
    std::shared_ptr<SpriteNode> node = std::dynamic_pointer_cast<SpriteNode>(dst);
    if (node) {
        node->_cols = _cols;
        node->_size = _size;
        node->_frame  = _frame;
        node->_bounds = _bounds;
    }
    return dst;
}


#pragma mark -
#pragma mark Attribute Accessors

/**
 * Sets the active frame as the given index.
 *
 * If the frame index is invalid, an error is raised.
 *
 * @param frame the index to make the active frame
 */
void SpriteNode::setFrame(int frame) {
    CUAssertLog(frame >= 0 && frame < _size, "Invalid animation frame %d", frame);
    
    _frame = frame;
    float x = (frame % _cols)*_bounds.size.width;
    float y = _texture->getSize().height - (1+frame/_cols)*_bounds.size.height;
    float dx = x-_bounds.origin.x;
    float dy = y-_bounds.origin.y;
    _bounds.origin.set(x,y);
    shiftTexture(dx, dy);
}

/**
 * Allocate the render data necessary to render this node.
 */
void SpriteNode::generateRenderData() {
    CUAssertLog(!_rendered, "Render data is already present");
    if (_texture == nullptr) {
        return;
    }
    
    _mesh.set(_polygon);
    _mesh.command = GL_TRIANGLES;
    
    // Antialias the boundaries (if required)
    if (_fringe > 0) {
        SimpleExtruder extruder;
        std::vector<std::vector<Uint32>> boundaries = _polygon.boundaries();
        Color4 clear = Color4(255,255,255,0);
        for(auto it = boundaries.begin(); it != boundaries.end(); ++it) {
            std::vector<Vec2> border;
            border.reserve(it->size());
            for (auto jt = it->begin(); jt != it->end(); ++jt) {
                border.push_back(_polygon.vertices[*jt]);
            }
            extruder.clear();
            extruder.set(border,true);
            extruder.setJoint(poly2::Joint::SQUARE);
            // Interior is to the left
            extruder.calculate(0,_fringe);
            extruder.getMesh(&_mesh,Color4::WHITE,clear);
        }
    }
    
    // Adjust the mesh as necesary
    Size nsize = getContentSize();
    Size bsize = _polygon.getBounds().size;
    
    Mat4 shift;
    bool adjust = false;
    if (nsize != bsize) {
        adjust = true;
        shift.scale((bsize.width > 0  ? nsize.width/bsize.width : 0),
                    (bsize.height > 0 ? nsize.height/bsize.height : 0), 1);
    }
    
    const Vec2 offset = _polygon.getBounds().origin;
    if (!_absolute && !offset.isZero()) {
        adjust = true;
        shift.translate(-offset.x,-offset.y,0);
    }
    
    // TODO: Figure out why SpriteNode needs this BEFORE texturing
    if (adjust) {
        _mesh *= shift;
    }

    _rendered = true;
    updateTextureCoords();
}

/**
 * Updates the texture coordinates for this polygon
 *
 * The texture coordinates are computed assuming that the polygon is
 * defined in image space, with the origin in the bottom left corner
 * of the texture.
 */
void SpriteNode::updateTextureCoords() {
    if (!_rendered) {
        return;
    }
    
    Size tsize = _texture->getSize();
    Vec2 off = _bounds.origin;
    for(auto it = _mesh.vertices.begin(); it != _mesh.vertices.end(); ++it) {
        float s = (it->position.x+off.x)/tsize.width;
        float t = 1-(it->position.y+off.y)/tsize.height;

        if (_flipHorizontal) {
            float x0 = off.x/tsize.width;
            float x1 = (off.x+_bounds.size.width)/tsize.width;
            s = x1-(s-x0);
        }
        if (_flipVertical)   {
            float y0 = off.y/tsize.height;
            float y1 = (off.y+_bounds.size.height)/tsize.height;
            t = y1-(t-y0);
        }

        it->texcoord.x = s*_texture->getMaxS()+(1-s)*_texture->getMinS();
        it->texcoord.y = t*_texture->getMaxT()+(1-t)*_texture->getMinT();
    
        if (_gradient) {
            // We have decided to do this on the content size, not the polygon
            s = (it->position.x)/_contentSize.width;
            t = (it->position.y)/_contentSize.height;

            if (_flipHorizontal) { s = 1-s; }
            if (_flipVertical)   { t = 1-t; }
            it->gradcoord.x = s;
            it->gradcoord.y = t;
        }
    }
}
