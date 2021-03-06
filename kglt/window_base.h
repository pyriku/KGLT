#ifndef KGLT_WINDOW_BASE_H
#define KGLT_WINDOW_BASE_H

#include <tr1/memory>

#include <sigc++/sigc++.h>

#include "keyboard.h"

#include "loaders/texture_loader.h"
#include "loaders/q2bsp_loader.h"
#include "loader.h"

#include "idle_task_manager.h"

#include "kazbase/logging/logging.h"
#include "kaztimer/kaztimer.h"

namespace kglt {
    
class WindowBase {
public:
    WindowBase():
        width_(0),
        height_(0),
        is_running_(true) {
        
        //Register the default resource loaders
        register_loader(LoaderType::ptr(new kglt::loaders::TextureLoaderType));
        register_loader(LoaderType::ptr(new kglt::loaders::Q2BSPLoaderType));

        ktiGenTimers(1, &timer_);
        ktiBindTimer(timer_);
        ktiStartGameTimer();
    }
    
    void init();

    virtual ~WindowBase() {
        
    }
    
    Loader::ptr loader_for(const std::string& filename, const std::string& type_hint) {
        std::string final_file = find_file(filename);

        //See if we can find a loader that supports this type hint
        for(LoaderType::ptr loader_type: loaders_) {
            if(loader_type->has_hint(type_hint) && loader_type->supports(filename)) {
                return loader_type->loader_for(final_file);
            }
        }

        throw std::runtime_error("Unable to find a loader for: " + filename);
    }

    Loader::ptr loader_for(const std::string& filename) {
        std::string final_file = find_file(filename);

        for(LoaderType::ptr loader_type: loaders_) {
            if(loader_type->supports(final_file) && !loader_type->requires_hint()) {
                return loader_type->loader_for(final_file);
            }
        }

        throw std::runtime_error("Unable to find a loader for: " + filename);
    }    
    
    void register_loader(LoaderType::ptr loader_type);
    void add_search_path(const std::string& path) {
        resource_paths_.push_back(path);
    }

    virtual sigc::signal<void, KeyCode>& signal_key_up() = 0;
    virtual sigc::signal<void, KeyCode>& signal_key_down() = 0;
    
    virtual void set_title(const std::string& title) = 0;
    virtual void cursor_position(int32_t& mouse_x, int32_t& mouse_y) = 0;
    
    virtual void check_events() = 0;
    virtual void swap_buffers() = 0;
    double delta_time() { return ktiGetDeltaTime(); }

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }
    
    Scene& scene();
    bool update();   

    IdleTaskManager& idle() { return idle_; }

protected:
    void stop_running() { is_running_ = false; }
    
    void set_width(uint32_t width) { 
        width_ = width; 
    }
    
    void set_height(uint32_t height) {
        height_ = height; 
    }
    
private:
    std::tr1::shared_ptr<Scene> scene_;
    uint32_t width_;
    uint32_t height_;

    std::vector<std::string> resource_paths_;
    std::vector<LoaderType::ptr> loaders_;
    bool is_running_;
    
    std::string find_file(const std::string& filename) {
        //FIXME: Search the resource paths!
        return filename;
    }
    
    IdleTaskManager idle_;

    KTIuint timer_;

    void destroy() {}

};

}

#endif
