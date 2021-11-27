#pragma once

struct Rigidbody;
struct MeshRenderable;

#define ENGINE_COMPONENTS \
    REGISTER_COMPONENT(Rigidbody) \
    REGISTER_COMPONENT(MeshRenderable) \