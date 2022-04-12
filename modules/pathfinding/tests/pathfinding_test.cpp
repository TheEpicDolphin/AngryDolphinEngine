
#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "../NavigationMesh.h"

template<typename T>
std::vector<T> _ConvertFrom(std::vector<T *>& input) 
{
    std::vector<T> output(input.size());
    std::transform(input.begin(), input.end(), output.begin(), [](T* t) { return *t; });
    return output;
}

void _WriteToObjectFile(const std::vector<pathfinding::NavigationMeshTileData>& tiles)
{
    std::ofstream objFile;
    objFile.open("navigation_mesh_generation.obj", std::ios::out);

    for (const pathfinding::NavigationMeshTileData& tileNavMeshData : tiles) {
        objFile << "\n# tile: ("
            << tileNavMeshData.tileCoordinates[0] << ", "
            << tileNavMeshData.tileCoordinates[1] << ")"
            << std::endl;
        for (int i = 0; i < tileNavMeshData.verticesCount * 3; i += 3) {
            objFile << "v "
                << tileNavMeshData.origin[0] + tileNavMeshData.vertices[i] << " "
                << tileNavMeshData.origin[1] + tileNavMeshData.vertices[i + 1] << " "
                << tileNavMeshData.origin[2] + tileNavMeshData.vertices[i + 2]
                << std::endl;
        }
    }

    int triangleIndicesOffset = 0;
    for (const pathfinding::NavigationMeshTileData& tileNavMeshData : tiles) {
        for (int i = 0; i < tileNavMeshData.trianglesCount * 3; i += 3) {
            objFile << "f "
                << triangleIndicesOffset + tileNavMeshData.triangles[i] + 1 << " "
                << triangleIndicesOffset + tileNavMeshData.triangles[i + 1] + 1 << " "
                << triangleIndicesOffset + tileNavMeshData.triangles[i + 2] + 1
                << std::endl;
        }
        triangleIndicesOffset += tileNavMeshData.verticesCount;
    }

    objFile.close();
}

void _PrintNavigationMeshTriangulation(const std::vector<pathfinding::NavigationMeshTileData>& tiles) {
    for (const pathfinding::NavigationMeshTileData& tileNavMeshData : tiles) {
        std::cout << "Tile Coordinates: (" 
            << tileNavMeshData.tileCoordinates[0] << " ," 
            << tileNavMeshData.tileCoordinates[1] << ")" << std::endl;
        std::cout << "Tile Origin: ("
            << tileNavMeshData.origin[0] << ", "
            << tileNavMeshData.origin[1] << ", "
            << tileNavMeshData.origin[2] << ")"
            << std::endl;
        std::cout << "TRIANGLES: " << tileNavMeshData.trianglesCount << std::endl;
        std::cout << "\n" << std::endl;
    }
}

TEST(pathfinding_test_suite, creation_test)
{
    pathfinding::NavigationMesh navigationMesh;
    float boundsMin[3] = { -1000, -1000, -1000 };
    float boundsMax[3] = { 1000, 1000, 1000 };
   
    //navigationMesh.initialize(60.0f, 0.5f, 1.5f, 0.5f, boundsMin, boundsMax);
    navigationMesh.initialize(60.0f, 0.5f, 1.5f, 0.5f);
    
    float transform[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    /*
    float transform[16] = {
        1, 0, 0, 0,
        0, 0.5f * sqrt(3), 0.5f, 0,
        0, -0.5f, 0.5f * sqrt(3), 0,
        0, 0, 0, 1,
    };
    */
    float vertices[] = {
        25, 30, 25,
        -25, 30, 25,
        -25, 0, -25,
        25, 0, -25
    };
    unsigned short indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    pathfinding::NavigationMeshGeometryEntityHandle handle;
    navigationMesh.registerNavigationMeshGeometryEntity(transform, indices, 2, vertices, 4, handle);
    std::cout << "Registered geometry entity with handle: " << handle << std::endl;

    pathfinding::NavigationMeshRegenerationChangeset changeset;
    navigationMesh.regenerateIfNeeded(changeset);

    std::cout << "Finished regenerating" << std::endl;
    std::cout << "Added:" << std::endl;
    _PrintNavigationMeshTriangulation(changeset.addedTiles);
    _WriteToObjectFile(changeset.addedTiles);
    std::cout << "Modifed:" << std::endl;
    _PrintNavigationMeshTriangulation(changeset.modifiedTiles);
    std::cout << "Removed:" << std::endl;
    _PrintNavigationMeshTriangulation(changeset.removedTiles);

    /*
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
            std::cout << "TRIANGLES:" << std::endl;
            for (int i = 0; i < addedTileRegenResults.navMeshVertices.size(); i+=9) {
                std::cout << "(" << addedTileRegenResults.navMeshVertices[i] << ", " << addedTileRegenResults.navMeshVertices[i + 1] << ", " << addedTileRegenResults.navMeshVertices[i + 2] << ")" << std::endl;
                std::cout << "(" << addedTileRegenResults.navMeshVertices[i + 3] << ", " << addedTileRegenResults.navMeshVertices[i + 4] << ", " << addedTileRegenResults.navMeshVertices[i + 5] << ")" << std::endl;
                std::cout << "(" << addedTileRegenResults.navMeshVertices[i + 6] << ", " << addedTileRegenResults.navMeshVertices[i + 7] << ", " << addedTileRegenResults.navMeshVertices[i + 8] << ")" << std::endl;
            }
        }
        std::cout << "Modifed:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults modifiedTileRegenResults : changeset.modifiedTiles) {
            std::cout << "(" << modifiedTileRegenResults.tx << " ," << modifiedTileRegenResults.ty << ")" << std::endl;
            std::cout << "TRIANGLES:" << std::endl;
            for (int i = 0; i < modifiedTileRegenResults.navMeshVertices.size(); i += 9) {
                std::cout << "(" << modifiedTileRegenResults.navMeshVertices[i] << ", " << modifiedTileRegenResults.navMeshVertices[i + 1] << ", " << modifiedTileRegenResults.navMeshVertices[i + 2] << ")" << std::endl;
                std::cout << "(" << modifiedTileRegenResults.navMeshVertices[i + 3] << ", " << modifiedTileRegenResults.navMeshVertices[i + 4] << ", " << modifiedTileRegenResults.navMeshVertices[i + 5] << ")" << std::endl;
                std::cout << "(" << modifiedTileRegenResults.navMeshVertices[i + 6] << ", " << modifiedTileRegenResults.navMeshVertices[i + 7] << ", " << modifiedTileRegenResults.navMeshVertices[i + 8] << ")" << std::endl;
            }
        }
        std::cout << "Removed:" << std::endl;
        for (pathfinding::NavigationMeshTileRegenerationResults removedTileRegenResults : changeset.removedTiles) {
            std::cout << "(" << removedTileRegenResults.tx << " ," << removedTileRegenResults.ty << ")" << std::endl;
        }
        });
        */
}

