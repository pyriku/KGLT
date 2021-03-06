#include <unittest++/UnitTest++.h>

#include <vector>

#include "kglt/shortcuts.h"
#include "kglt/kglt.h"
#include "kglt/object.h"

using namespace kglt;

TEST(test_overlay_creation) {

}

TEST(test_deleting_overlays_deletes_children) {
    kglt::Window window;
    kglt::Scene& scene = window.scene();

    kglt::OverlayID oid = scene.new_overlay(); //Create the overlay
    kglt::MeshID cid1 = scene.new_mesh(&scene.overlay(oid)); //Create a child
    kglt::MeshID cid2 = scene.new_mesh(&scene.mesh(cid1)); //Crete a child of the child

    CHECK_EQUAL(1, scene.overlay(oid).child_count());
    CHECK_EQUAL(1, scene.mesh(cid1).child_count());
    CHECK_EQUAL(0, scene.mesh(cid2).child_count());

    scene.delete_overlay(oid);
    CHECK(!scene.has_overlay(oid));
    CHECK(!scene.has_mesh(cid1));
    CHECK(!scene.has_mesh(cid2));
}
