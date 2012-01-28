#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <tr1/memory>

#include "kazmath/vec3.h"
#include "kazmath/quaternion.h"
#include "types.h"

namespace kglt {

class ObjectVisitor;

class Object {
private:
    static uint64_t object_counter;
    uint64_t id_;

    kmVec3 position_;
    kmQuaternion rotation_;

protected:
    Object* parent_;

    std::vector<Object*> children_;

    void attach_child(Object* child) {
        if(child->has_parent()) {
            child->parent().detach_child(child);
        }

        child->parent_ = this;
        children_.push_back(child);
    }

    void detach_child(Object* child) {
        //Make sure that the child has a parent
        if(!child->has_parent()) {
            return;
        }

        //Make sure *we* are the parent
        if(&child->parent() != this) {
            return;
        }

        //Erase the child from our children and set its parent to null
        child->parent_ = nullptr;
        children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end());
    }

    float yaw_;

public:
    typedef std::tr1::shared_ptr<Object> ptr;

    Object():
        id_(++object_counter),
        parent_(nullptr),
        yaw_(0.0f) {

        kmQuaternionIdentity(&rotation_); //Set a default rotation
    }

    virtual ~Object();

    virtual void move_to(float x, float y, float z);
    virtual void move_forward(float amount);
    virtual void rotate_y(float amount);

    void set_parent(Object* p) {
        if(has_parent()) {
            parent().detach_child(this);
        }

        if(p) {
            p->attach_child(this);
        }

    }

    bool has_parent() const { return parent_ != nullptr; }
    Object& parent() { assert(parent_); return *parent_; }
    Object& child(uint32_t i);

    kmVec3& position() { return position_; }
    kmQuaternion& rotation() { return rotation_; }

    template<typename T>
    Object* root_as() {
        Object* self = this;
        while(self->has_parent()) {
            self = &self->parent();
        }

        return dynamic_cast<T>(self);
    }

    virtual void accept(ObjectVisitor& visitor) = 0;

    uint64_t id() const { return id_; }
};

}

#endif // OBJECT_H_INCLUDED