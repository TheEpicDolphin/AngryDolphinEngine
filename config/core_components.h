#pragma once

// TODO: Make a version of this file for game components (made by user that they fill out), and another for all
// the components in the modules subdirectory.

struct MeshRenderableComponent;
struct CameraComponent;
struct RigidbodyComponent;

#define FOREACH_CORE_COMPONENT_TYPE(ACTION) \
    ACTION(MeshRenderableComponent) \
    ACTION(CameraComponent) \
    ACTION(RigidbodyComponent)