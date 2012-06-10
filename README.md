# Kazade's GL Toolkit

## What is it?

While working on reverse engineering some obscure 3D model format, I needed a 
quick way to test file parsing, and procedually generating and rendering a mesh
from the model file. This library was spawned from that.

## How do I use it?

Well, it's pretty simple really...

```
#include "GL/window.h"

int main(int argc, char* argv[]) {

    kglt::Window window;
    
    while(window.update()) {}    

    return 0;
}
```

Crazy eh? Those two lines of code construct an OpenGL window, with an empty
scene and the program runs until the window is closed. 

But you wanted something more interesting than that right? OK, let's draw a
wireframe triangle:

    #include "GL/window.h"

    int main(int argc, char* argv[]) {

        kglt::Window window;

        //Set some rendering options
        window.scene().render_options.wireframe_enabled = true;

        //Create a new mesh
        kglt::Mesh& mesh = kglt::return_new_mesh(window.scene());
    
        //Add a submesh, add 3 vertices and a triangle made up of them
        mesh.add_submesh();
        mesh.submesh().add_vertex(-1.0, 0.0, 0.0);
        mesh.submesh().add_vertex(0.0, 1.0, 0.0);
        mesh.submesh().add_vertex(1.0, 0.0, 0.0);
        mesh.submesh().add_triangle(0, 1, 2);
    
        while(window.update()) {}    

        return 0;
    }


We can also draw a rectangle even more easily:


    #include "GL/window.h"

    int main(int argc, char* argv[]) {

        kglt::Window window;

        //Set some rendering options
        window.scene().render_options.wireframe_enabled = true;

        //Create a new mesh
        kglt::Mesh& mesh = kglt::return_new_mesh(window.scene());
    
        //Construct a rectangle 1.0 unit across, and half a unit high
        kglt::procedural::mesh::rectangle(mesh, 1.0, 0.5);
	
        //Move the rectangle out a little
        mesh.move_to(0.0, 0.0, -1.0);
	
        //Create an orthographic projection, 2 units high, with a 16:9 ratio (default)
        window.scene().viewport().set_orthographic_projection_from_height(2.0);
	
        while(window.update()) {}    

        return 0;
    }


# Documentation

## The Scene

### Passes

## Backgrounds

Each Scene has a background() property. Backgrounds are made up of one or more image layers that can be independently scrolled. Each layer is always positioned central to the viewport, and is rendered at the resolution of the image. You can clip the viewport to a certain section of a background by using set_visible_dimensions(X, Y);

For example, if you load a background layer from an image with a resolution of 512x256. You could then set the visible dimensions to 320x224 and use background().layer(0).scroll_x(amount) to move the viewport around.

Current constraints:

 * All layers must be the same dimensions
 * You cannot set the visible dimensions to greater than the layer size






