
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

    navigationMesh.regenerateIfNeeded([](pathfinding::NavigationMeshRegenerationChangeset changeset) {
        std::cout << "Finished regenerating" << std::endl;
        std::cout << "Added:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults addedTileRegenResults : changeset.addedTiles) {
            std::cout << "(" << addedTileRegenResults.tx << " ," << addedTileRegenResults.ty << ")" << std::endl;
        }
        std::cout << "Modifed:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults modifiedTileRegenResults: changeset.modifiedTiles) {
            std::cout << "(" << modifiedTileRegenResults.tx << " ," << modifiedTileRegenResults.ty << ")" << std::endl;
        }
        std::cout << "Removed:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults removedTileRegenResults : changeset.removedTiles) {
            std::cout << "(" << removedTileRegenResults.tx << " ," << removedTileRegenResults.ty << ")" << std::endl;
        }
    });

    float newTransform[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        1, 0, 1, 1,
    };
    navigationMesh.setNavigationMeshGeometryEntityTransform(handle, newTransform);

    navigationMesh.regenerateIfNeeded([](pathfinding::NavigationMeshRegenerationChangeset changeset) {
        std::cout << "Finished regenerating" << std::endl;
        std::cout << "Added:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults addedTileRegenResults : changeset.addedTiles) {
            std::cout << "(" << addedTileRegenResults.tx << " ," << addedTileRegenResults.ty << ")" << std::endl;
        }
        std::cout << "Modifed:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults modifiedTileRegenResults: changeset.modifiedTiles) {
            std::cout << "(" << modifiedTileRegenResults.tx << " ," << modifiedTileRegenResults.ty << ")" << std::endl;
        }
        std::cout << "Removed:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults removedTileRegenResults : changeset.removedTiles) {
            std::cout << "(" << removedTileRegenResults.tx << " ," << removedTileRegenResults.ty << ")" << std::endl;
        }
    });

    navigationMesh.unregisterNavigationMeshGeometryEntity(handle);

    navigationMesh.regenerateIfNeeded([](pathfinding::NavigationMeshRegenerationChangeset changeset) {
        std::cout << "Finished regenerating" << std::endl;
        std::cout << "Added:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults addedTileRegenResults : changeset.addedTiles) {
            std::cout << "(" << addedTileRegenResults.tx << " ," << addedTileRegenResults.ty << ")" << std::endl;
        }
        std::cout << "Modifed:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults modifiedTileRegenResults : changeset.modifiedTiles) {
            std::cout << "(" << modifiedTileRegenResults.tx << " ," << modifiedTileRegenResults.ty << ")" << std::endl;
        }
        std::cout << "Removed:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults removedTileRegenResults : changeset.removedTiles) {
            std::cout << "(" << removedTileRegenResults.tx << " ," << removedTileRegenResults.ty << ")" << std::endl;
        }
        });
}

