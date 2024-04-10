#ifndef __STATIC_FILTH_C__
#define __STATIC_FILTH_C__

#include "StaticFilth.h"

using namespace cugl;

/** Use this consturctor to generate static filth on window board */
StaticFilth::StaticFilth(const cugl::Vec2 p) {
    boardPosition = p;
}

// draws a static filth on the screen on window pane at location windowPos
void StaticFilth::drawStatic(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Size size, cugl::Affine2 filthTrans) {
        // calculate scale and size of filth drawing in reference to a window pane so that it is centered
        // scale applied to each filth tile

        // loop over all grid points and draw window panes and filth
        batch->draw(_filthStaticTexture, Vec2(), filthTrans);
//        CULog("filth added");
}

#endif
