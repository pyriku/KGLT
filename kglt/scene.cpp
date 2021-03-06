#include "glee/GLee.h"
#include "scene.h"
#include "renderer.h"
#include "ui.h"
#include "partitioners/null_partitioner.h"
#include "shaders/default_shaders.h"

namespace kglt {

Scene::Scene(WindowBase* window):
    Object(nullptr),
    active_camera_(DefaultCameraID),
    window_(window),
    default_texture_(0),
    default_shader_(0),
    default_material_(0),
    ambient_light_(1.0, 1.0, 1.0, 1.0),
    background_(this),
    ui_interface_(new UI(this)),
    partitioner_(new NullPartitioner(*this)) {

    TemplatedManager<Scene, Mesh, MeshID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Mesh, MeshID>));
    TemplatedManager<Scene, Camera, CameraID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Camera, CameraID>));
    TemplatedManager<Scene, Text, TextID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Text, TextID>));
    TemplatedManager<Scene, ShaderProgram, ShaderID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_shader_callback));

    background().set_parent(this);

    active_camera_ = new_camera(); //Create a default camera

    //Set up the default render options
    render_options.wireframe_enabled = false;
    render_options.texture_enabled = true;
    render_options.backface_culling_enabled = true;
    render_options.point_size = 1;

    /**
     * Create the default pass, which uses a perspective projection and
     * a fullscreen viewport
     */
     add_pass(GenericRenderer::create(render_options), VIEWPORT_TYPE_FULL);
}

Scene::~Scene() {
    //TODO: Log the unfreed resources (textures, meshes, materials etc.)
}

void Scene::initialize_defaults() {
    //Create the default blank texture
    default_texture_ = new_texture();
    Texture& tex = texture(default_texture_);
    tex.resize(1, 1);
    tex.set_bpp(32);

    tex.data()[0] = 255;
    tex.data()[1] = 255;
    tex.data()[2] = 255;
    tex.data()[3] = 255;
    tex.upload();

    //Create the default shader program
    default_shader_ = new_shader();
    ShaderProgram& def = shader(default_shader_); //Create a default shader;

    assert(glGetError() == GL_NO_ERROR);

    def.add_and_compile(SHADER_TYPE_VERTEX, ambient_render_vert);
    def.add_and_compile(SHADER_TYPE_FRAGMENT, ambient_render_frag);
    def.activate();

    def.params().register_auto(SP_AUTO_MODELVIEW_PROJECTION_MATRIX, "modelview_projection_matrix");
    def.params().register_auto(SP_AUTO_LIGHT_GLOBAL_AMBIENT, "global_ambient");

    //Bind the vertex attributes for the default shader and relink
    def.params().register_attribute(SP_ATTR_VERTEX_POSITION, "vertex_position");
    def.params().register_attribute(SP_ATTR_VERTEX_TEXCOORD0, "vertex_texcoord_1");
    def.params().register_attribute(SP_ATTR_VERTEX_COLOR, "vertex_diffuse");
    //def.params().register_attribute(SP_ATTR_VERTEX_NORMAL, "vertex_normal");

    def.params().set_int("texture_1", 0); //Set texture_1 to be the first texture unit

    def.relink();

    phong_shader_ = new_shader();
    ShaderProgram& phong = shader(phong_shader_);
    phong.add_and_compile(SHADER_TYPE_VERTEX, phong_lighting_vert);
    phong.add_and_compile(SHADER_TYPE_FRAGMENT, phong_lighting_frag);
    phong.activate();
    
    phong.params().register_auto(SP_AUTO_MODELVIEW_PROJECTION_MATRIX, "modelview_projection_matrix");
    phong.params().register_auto(SP_AUTO_LIGHT_POSITION, "light_position");
    phong.params().register_auto(SP_AUTO_LIGHT_AMBIENT, "light_ambient");
    phong.params().register_auto(SP_AUTO_LIGHT_SPECULAR, "light_specular");
    phong.params().register_auto(SP_AUTO_LIGHT_DIFFUSE, "light_diffuse");
    phong.params().register_auto(SP_AUTO_LIGHT_CONSTANT_ATTENUATION, "light_constant_attenuation");
    phong.params().register_auto(SP_AUTO_LIGHT_LINEAR_ATTENUATION, "light_linear_attenuation");
    phong.params().register_auto(SP_AUTO_LIGHT_QUADRATIC_ATTENUATION, "light_quadratic_attenuation");

    phong.params().register_attribute(SP_ATTR_VERTEX_POSITION, "vertex_position");
    phong.params().register_attribute(SP_ATTR_VERTEX_NORMAL, "vertex_normal");
    phong.relink();

    //Finally create the default material to link them
    default_material_ = new_material();
    Material& mat = material(default_material_);
    mat.technique().new_pass(default_shader_);
    mat.technique().new_pass(phong_shader_);
    
    mat.technique().pass(0).set_texture_unit(0, default_texture_);
    mat.technique().pass(0).set_iteration(ITERATE_ONCE);
    mat.technique().pass(1).set_iteration(ITERATE_ONCE_PER_LIGHT, 8);
}

MeshID Scene::new_mesh(Object *parent) {
    MeshID result = TemplatedManager<Scene, Mesh, MeshID>::manager_new();
    if(parent) {
        mesh(result).set_parent(parent);
    }

    //Add the mesh to the partitioner
    partitioner_->add(mesh(result));

    return result;
}

bool Scene::has_mesh(MeshID m) const {
    return TemplatedManager<Scene, Mesh, MeshID>::manager_contains(m);
}

Mesh& Scene::mesh(MeshID m) {
    return TemplatedManager<Scene, Mesh, MeshID>::manager_get(m);
}

void Scene::delete_mesh(MeshID mid) {
    //Remove the mesh from the partitioner
    partitioner_->remove(mesh(mid));

    Mesh& obj = mesh(mid);
    obj.destroy_children();
    return TemplatedManager<Scene, Mesh, MeshID>::manager_delete(mid);
}

MaterialID Scene::new_material(MaterialID clone_from) {
    MaterialID result = TemplatedManager<Scene, Material, MaterialID>::manager_new();
    if(clone_from > 0) {
        kglt::Material& existing = material(clone_from);
        kglt::Material& new_mat = material(result);

        for(uint32_t i = 0; i < existing.technique_count(); ++i) {
            //FIXME: handle multiple schemes
            MaterialTechnique& old_tech = existing.technique();
            MaterialTechnique& new_tech = new_mat.technique();
            for(uint32_t j = 0; j < old_tech.pass_count(); ++j) {
                MaterialPass& old_pass = old_tech.pass(j);
                uint32_t pass_id = new_tech.new_pass(old_pass.shader());
                MaterialPass& new_pass = new_tech.pass(pass_id);

                new_pass.set_iteration(old_pass.iteration(), old_pass.max_iterations());
                for(uint32_t k = 0; k < old_pass.texture_unit_count(); ++k) {
                    new_pass.set_texture_unit(k, old_pass.texture_unit(k).texture());
                }
                //FIXME: Copy animated texture unit and other properties
            }
        }
    }
    return result;
}

Material& Scene::material(MaterialID mid) {
    return TemplatedManager<Scene, Material, MaterialID>::manager_get(mid);
}

void Scene::delete_material(MaterialID mid) {
    TemplatedManager<Scene, Material, MaterialID>::manager_delete(mid);
}

TextureID Scene::new_texture() {
    return TemplatedManager<Scene, Texture, TextureID>::manager_new();
}

void Scene::delete_texture(TextureID tid) {
    TemplatedManager<Scene, Texture, TextureID>::manager_delete(tid);
}

Texture& Scene::texture(TextureID t) {
    return TemplatedManager<Scene, Texture, TextureID>::manager_get(t);
}

CameraID Scene::new_camera() {
    return TemplatedManager<Scene, Camera, CameraID>::manager_new();
}

Camera& Scene::camera(CameraID c) {
    return TemplatedManager<Scene, Camera, CameraID>::manager_get(c);
}

void Scene::delete_camera(CameraID cid) {
    Camera& obj = camera(cid);
    obj.destroy_children();
    TemplatedManager<Scene, Camera, CameraID>::manager_delete(cid);
}

ShaderProgram& Scene::shader(ShaderID s) {
    return TemplatedManager<Scene, ShaderProgram, ShaderID>::manager_get(s);
}

ShaderID Scene::new_shader() {
    return TemplatedManager<Scene, ShaderProgram, ShaderID>::manager_new();
}

void Scene::delete_shader(ShaderID s) {
    TemplatedManager<Scene, ShaderProgram, ShaderID>::manager_delete(s);
}

FontID Scene::new_font() {
    return TemplatedManager<Scene, Font, FontID>::manager_new();
}

Font& Scene::font(FontID f) {
    return TemplatedManager<Scene, Font, FontID>::manager_get(f);
}

void Scene::delete_font(FontID f) {
    TemplatedManager<Scene, Font, FontID>::manager_delete(f);
}

TextID Scene::new_text() {
    return TemplatedManager<Scene, Text, TextID>::manager_new();
}

Text& Scene::text(TextID t) {
    return TemplatedManager<Scene, Text, TextID>::manager_get(t);
}

const Text& Scene::text(TextID t) const {
    return TemplatedManager<Scene, Text, TextID>::manager_get(t);
}

void Scene::delete_text(TextID tid) {
    Text& obj = text(tid);
    obj.destroy_children();
    TemplatedManager<Scene, Text, TextID>::manager_delete(tid);
}

OverlayID Scene::new_overlay() {
    return TemplatedManager<Scene, Overlay, OverlayID>::manager_new();
}

bool Scene::has_overlay(OverlayID o) const {
    return TemplatedManager<Scene, Overlay, OverlayID>::manager_contains(o);
}

Overlay& Scene::overlay(OverlayID overlay) {
    return TemplatedManager<Scene, Overlay, OverlayID>::manager_get(overlay);
}

void Scene::delete_overlay(OverlayID oid) {
    Overlay& obj = overlay(oid);
    obj.destroy_children();
    TemplatedManager<Scene, Overlay, OverlayID>::manager_delete(oid);
}

LightID Scene::new_light(Object* parent, LightType type) {
    LightID lid = TemplatedManager<Scene, Light, LightID>::manager_new();

    Light& l = light(lid);
    l.set_type(type);
    if(parent) {
        l.set_parent(parent);
    }

    partitioner_->add(l); //Add the light to the partitioner

    return lid;
}

Light& Scene::light(LightID light_id) {
    return TemplatedManager<Scene, Light, LightID>::manager_get(light_id);
}

void Scene::delete_light(LightID light_id) {
    Light& obj = light(light_id);
    partitioner_->remove(obj); //Remove the light from the partitioner
    obj.destroy_children();
    TemplatedManager<Scene, Light, LightID>::manager_delete(light_id);
}

void Scene::init() {
    assert(glGetError() == GL_NO_ERROR);

    initialize_defaults();

}

std::pair<ShaderID, bool> Scene::find_shader(const std::string& name) {
    std::map<std::string, ShaderID>::const_iterator it = shader_lookup_.find(name);
    if(it == shader_lookup_.end()) {
        return std::make_pair(0, false);
    }

    return std::make_pair((*it).second, true);
}

void Scene::update(double dt) {
    /*
      Update all animated materials
    */
    for(std::pair<MaterialID, Material::ptr> p: generic::TemplatedManager<Scene, Material, MaterialID>::objects_) {
        p.second->update(dt);
    }

    for(uint32_t i = 0; i < child_count(); ++i) {
        Object& c = child(i);
        c.update(dt);
    }
}

void Scene::render() {
    /**
     * Go through all the render passes
     * set the render options and send the viewport to OpenGL
     * before going through the scene, objects in the scene
     * should be able to mark as only being renderered in certain
     * passes
     */
    for(Pass& pass: passes_) {
        pass.renderer().set_options(render_options);
        pass.viewport().update_opengl();

        signal_render_pass_started_(pass);
        pass.renderer().render(*this);
        signal_render_pass_finished_(pass);
    }
}

MeshID Scene::_mesh_id_from_mesh_ptr(Mesh* mesh) {
    return TemplatedManager<Scene, Mesh, MeshID>::_get_object_id_from_ptr(mesh);
}

}
