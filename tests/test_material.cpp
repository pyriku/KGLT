
#include <unittest++/UnitTest++.h>

#include <vector>

#include "kglt/shortcuts.h"
#include "kglt/kglt.h"
#include "kglt/object.h"

using namespace kglt;

TEST(test_material_initialization) {
    kglt::Window window;
    kglt::Scene& scene = window.scene();

    Material& mat = scene.material(scene.new_material());

    CHECK_EQUAL(1, mat.technique_count()); //Should return the default technique
    CHECK_EQUAL(kglt::DEFAULT_MATERIAL_SCHEME, mat.technique().scheme());
    mat.technique().new_pass(0); //Create a pass
    CHECK_EQUAL(1, mat.technique().pass_count()); //Should return the default pass
    CHECK(kglt::Colour::white == mat.technique().pass(0).diffuse()); //Check the default pass sets white as the default
    CHECK(kglt::Colour::white == mat.technique().pass(0).ambient()); //Check the default pass sets white as the default
    CHECK(kglt::Colour::white == mat.technique().pass(0).specular()); //Check the default pass sets white as the default
    CHECK_EQUAL(0.0, mat.technique().pass(0).shininess());
}

TEST(test_material_applies_to_mesh) {
    kglt::Window window;
    kglt::Scene& scene = window.scene();

    MaterialID mid = scene.new_material();
    MeshID mesh_id = scene.new_mesh();
    Mesh& mesh = scene.mesh(mesh_id);
    mesh.apply_material(mid);
    CHECK_EQUAL(mid, mesh.material());
}
