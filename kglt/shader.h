#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <set>
#include <string>
#include <tr1/memory>

#include "kazbase/list_utils.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include "kazmath/mat4.h"
#include "kazmath/mat3.h"
#include "kazmath/vec3.h"

#include "loadable.h"
#include "generic/identifiable.h"
#include "types.h"

namespace kglt {

class Scene;

/*
  Automatic uniforms that are set by the renderer
*/
enum ShaderAvailableAuto {
    SP_AUTO_MODELVIEW_PROJECTION_MATRIX,
    SP_AUTO_MODELVIEW_MATRIX,
    SP_AUTO_PROJECTION_MATRIX,
    SP_AUTO_MATERIAL_DIFFUSE,
    SP_AUTO_MATERIAL_SPECULAR,
    SP_AUTO_MATERIAL_AMBIENT,
    SP_AUTO_MATERIAL_SHININESS,
    SP_AUTO_LIGHT_GLOBAL_AMBIENT,
    SP_AUTO_LIGHT_POSITION,
    SP_AUTO_LIGHT_DIRECTION,
    SP_AUTO_LIGHT_DIFFUSE,
    SP_AUTO_LIGHT_SPECULAR,
    SP_AUTO_LIGHT_AMBIENT,
    SP_AUTO_LIGHT_CONSTANT_ATTENUATION,
    SP_AUTO_LIGHT_LINEAR_ATTENUATION,
    SP_AUTO_LIGHT_QUADRATIC_ATTENUATION

    //TODO: cameras(?)
};

const std::set<ShaderAvailableAuto> SHADER_AVAILABLE_AUTOS = {
    SP_AUTO_MODELVIEW_PROJECTION_MATRIX,
    SP_AUTO_MODELVIEW_MATRIX,
    SP_AUTO_PROJECTION_MATRIX,
    SP_AUTO_MATERIAL_DIFFUSE,
    SP_AUTO_MATERIAL_SPECULAR,
    SP_AUTO_MATERIAL_AMBIENT,
    SP_AUTO_MATERIAL_SHININESS
};

enum ShaderAvailableAttributes {
    SP_ATTR_VERTEX_POSITION,
    SP_ATTR_VERTEX_DIFFUSE,
    SP_ATTR_VERTEX_NORMAL,
    SP_ATTR_VERTEX_TEXCOORD0,
    SP_ATTR_VERTEX_COLOR = SP_ATTR_VERTEX_DIFFUSE
};

const std::set<ShaderAvailableAttributes> SHADER_AVAILABLE_ATTRS = {
    SP_ATTR_VERTEX_POSITION,
    SP_ATTR_VERTEX_DIFFUSE,
    SP_ATTR_VERTEX_NORMAL,
    SP_ATTR_VERTEX_TEXCOORD0
};

class ShaderProgram;

class ShaderParams {
public:
    ShaderParams(ShaderProgram& parent);

    void register_auto(ShaderAvailableAuto auto_const, const std::string& uniform_name);
    void register_attribute(ShaderAvailableAttributes attr_const, const std::string& attrib_name);

    void set_int(const std::string& uniform_name, const int32_t value);
    void set_float(const std::string& uniform_name, const float value);
    void set_mat4x4(const std::string& uniform_name, const kmMat4& values);
    void set_mat3x3(const std::string& uniform_name, const kmMat3& values);
    void set_vec3(const std::string& uniform_name, const kmVec3& values);
    void set_vec4(const std::string& uniform_name, const kmVec4& values);
    void set_colour(const std::string& uniform_name, const Colour& values);

    bool uses_auto(ShaderAvailableAuto auto_const) const { return container::contains(auto_uniforms_, auto_const); }
    bool uses_attribute(ShaderAvailableAttributes attr_const) const { return container::contains(auto_attributes_, attr_const); }

    std::string auto_uniform_variable_name(ShaderAvailableAuto auto_name) const {
        auto it = auto_uniforms_.find(auto_name);
        if(it == auto_uniforms_.end()) {
            throw std::logic_error("Specified auto is not registered");
        }

        return (*it).second;
    }

    std::string attribute_variable_name(ShaderAvailableAttributes attr_name) const {
        auto it = auto_attributes_.find(attr_name);
        if(it == auto_attributes_.end()) {
            throw std::logic_error("Specified attribute is not registered");
        }

        return (*it).second;
    }

private:
    ShaderProgram& program_;

    std::map<ShaderAvailableAuto, std::string> auto_uniforms_;
    std::map<ShaderAvailableAttributes, std::string> auto_attributes_;
};

enum ShaderType {
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT,
    SHADER_TYPE_MAX
};

class ShaderProgram :
    public Loadable,
    public generic::Identifiable<ShaderID> {

public:
    typedef std::tr1::shared_ptr<ShaderProgram> ptr;

    ShaderProgram(Scene* scene, ShaderID id);
    ~ShaderProgram();

    void activate();
    void add_and_compile(ShaderType type, const std::string& source);

    void relink();

    ShaderParams& params() { return params_; }

    int32_t get_attrib_loc(const std::string& name);

    void bind_attrib(uint32_t idx, const std::string& name);
    int32_t get_uniform_loc(const std::string& name);
    bool has_uniform(const std::string& name);

private:
    void set_uniform(const std::string& name, const float x);
    void set_uniform(const std::string& name, const int32_t x);
    void set_uniform(const std::string& name, const kmMat4* matrix);
    void set_uniform(const std::string& name, const kmMat3* matrix);
    void set_uniform(const std::string& name, const kmVec3* vec);
    void set_uniform(const std::string& name, const kmVec4* vec);

    ShaderProgram(const ShaderProgram& rhs);
    ShaderProgram& operator=(const ShaderProgram& rhs);

    uint32_t program_id_;
    uint32_t shader_ids_[SHADER_TYPE_MAX];

    std::map<std::string, int32_t> cached_uniform_locations_;

    ShaderParams params_;

    friend class ShaderParams;
};

}

#endif // SHADER_H_INCLUDED
