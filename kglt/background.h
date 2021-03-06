#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <string>
#include <vector>
#include <tr1/memory>
#include <cstdint>
#include <kazmath/mat4.h>

#include "object.h"
#include "types.h"
#include "generic/visitor.h"

namespace kglt {

class Scene;
class Background;
class Renderer;

class BackgroundLayer {
public:
    BackgroundLayer(Background& background, const std::string& image_path);
    ~BackgroundLayer();

    void scroll_x(double amount);
    void scroll_y(double amount);

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }

    MaterialID material_id() const { return material_id_; }
    MeshID mesh_id() const { return mesh_id_; }

    Background& background() { return background_; }

private:
    Background& background_;
    TextureID texture_id_;
    MaterialID material_id_;
    MeshID mesh_id_;

    uint32_t width_;
    uint32_t height_;

    double offset_x_;
    double offset_y_;
};

class Background :
    public Object {

public:
    VIS_DEFINE_VISITABLE();

    Background(Scene *scene);

    void add_layer(const std::string& image_path);
    BackgroundLayer& layer(uint32_t index) { return *layers_.at(index); }
    uint32_t layer_count() const { return layers_.size(); }

    void set_visible_dimensions(double width, double height);

    double visible_x() const { return visible_x_; }
    double visible_y() const { return visible_y_; }

private:
    std::vector<std::tr1::shared_ptr<BackgroundLayer> > layers_;
    double visible_x_;
    double visible_y_;

    kmMat4 tmp_projection_;

    void destroy() {}
};

}
#endif // BACKGROUND_H
