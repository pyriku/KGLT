#ifndef KGLT_VIEWPORT_H
#define KGLT_VIEWPORT_H

#include <cstdint>
#include "kazmath/mat4.h"
#include "types.h"

namespace kglt {

class Scene;

enum AspectRatio {
    ASPECT_RATIO_CUSTOM,
    ASPECT_RATIO_4_BY_3,
    ASPECT_RATIO_16_BY_9,
    ASPECT_RATIO_16_BY_10
};

enum ProjectionType {
    PROJECTION_TYPE_PERSPECTIVE,
    PROJECTION_TYPE_ORTHOGRAPHIC
};

enum ViewportType {
	VIEWPORT_TYPE_FULL,	
	VIEWPORT_TYPE_BLACKBAR_4_BY_3,
	VIEWPORT_TYPE_BLACKBAR_16_BY_9,
	VIEWPORT_TYPE_BLACKBAR_16_BY_10,
	VIEWPORT_TYPE_VERTICAL_SPLIT_LEFT,
	VIEWPORT_TYPE_VERTICAL_SPLIT_RIGHT,
	VIEWPORT_TYPE_HORIZONTAL_SPLIT_TOP,
	VIEWPORT_TYPE_HORIZONTAL_SPLIT_BOTTOM,
	VIEWPORT_TYPE_CUSTOM
};

class Viewport {
public:
    Viewport(Scene* parent);

	void configure(ViewportType viewport);

    void set_size(uint32_t width, uint32_t height);
    void set_position(uint32_t left, uint32_t top);

	void set_background_colour(const Colour& colour) {
		colour_ = colour;
	}

	void update_opengl() const;

private:
	Scene* parent_;
	
    uint32_t x_;
    uint32_t y_;
    uint32_t width_;
    uint32_t height_;
    
	ViewportType type_;
	Colour colour_;
};

}

#endif
