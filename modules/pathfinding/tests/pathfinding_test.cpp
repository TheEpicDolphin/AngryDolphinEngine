
#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <iostream>

#include "../NavigationMesh.h"

template<typename T>
std::vector<T> _ConvertFrom(std::vector<T *>& input) 
{
    std::vector<T> output(input.size());
    std::transform(input.begin(), input.end(), output.begin(), [](T* t) { return *t; });
    return output;
}

TEST(pathfinding_test_suite, creation_test)
{
    pathfinding::NavigationMesh navigationMesh;
    navigationMesh.initialize(45.0f, 0.5f, 1.5f, 0.25f);

    float transform[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    float vertices[] = {
        5, 0, 5,
        -5, 0, 5,
        -5, 0, -5,
        5, 0, -5
    };
    unsigned short indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    pathfinding::NavigationMeshGeometryEntityHandle handle;
    navigationMesh.registerNavigationMeshGeometryEntity(transform, indices, 2, vertices, 4, handle);
    std::cout << "Registered geometry entity with handle: " << handle << std::endl;

    navigationMesh.regenerateIfNeeded([](std::vector<pathfinding::TileRegenerationResults> tileResults) {
        std::cout << "Finished regenerating" << std::endl;
        for (pathfinding::TileRegenerationResults tileResult : tileResults) {
            std::cout << "(" << tileResult.tx << " ," << tileResult.ty << ")" << std::endl;

        }
    });
}

