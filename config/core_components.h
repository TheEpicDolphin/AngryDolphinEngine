#pragma once

// TODO: Make a version of this file for game components (made by user that they fill out), and another for all
// the components in the modules subdirectory.

struct MeshRenderableComponent;
struct CameraComponent;
struct RigidbodyComponent;

#define CORE_COMPONENTS \
    REGISTER_COMPONENT(MeshRenderableComponent) \
    REGISTER_COMPONENT(CameraComponent) \
    REGISTER_COMPONENT(RigidbodyComponent)