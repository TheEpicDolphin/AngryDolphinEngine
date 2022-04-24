
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

void _WriteToObjectFile(const std::vector<pathfinding::NavigationMeshTileData>& tiles, const float* pathPoints, uint32_t numPathPoints)
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

    if (numPathPoints > 0) {
        // Draw path points, if any.
        objFile << "\n# Path points" << std::endl;
        for (int i = 0; i < numPathPoints * 3; i+=3) {
            objFile
                << "v " << pathPoints[i] << " " << pathPoints[i + 1] << " " << pathPoints[i + 2] << std::endl
                << "v " << pathPoints[i] << " " << pathPoints[i + 1] + 0.5f << " " << pathPoints[i + 2] << std::endl;
        }
    }

    objFile << "\n# triangle face elements" << std::endl;
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

    if (numPathPoints > 0) {
        objFile << "\n# path line elements" << std::endl;
        for (int i = 0; i < numPathPoints; ++i) {
            int n = triangleIndicesOffset + (2 * i) + 1;
            if (i > 0) {
                objFile << "f " << n << " " << n - 2 << " " << n + 1 << std::endl;
            }
            if (i < (numPathPoints - 1)) {
                objFile << "f " << n << " " << n + 1 << " " << n + 3 << std::endl;
            }
        }
        objFile << std::endl;
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
    std::cout << "Modifed:" << std::endl;
    _PrintNavigationMeshTriangulation(changeset.modifiedTiles);
    std::cout << "Removed:" << std::endl;
    _PrintNavigationMeshTriangulation(changeset.removedTiles);


    //float fromPoint[3] = { -30, 30, 30 };
    float fromPoint[3] = { -8, 18, 15 };
    //float toPoint[3] = { 30, -5, -30 };
    float toPoint[3] = { 8, 23, 8 };
    float pathPoints[300];
    uint32_t foundPathPointsCount;
    navigationMesh.findPath(fromPoint, toPoint, 100, pathPoints, foundPathPointsCount);
    for (uint32_t i = 0; i < 3 * foundPathPointsCount; i+=3) {
        float* pathPoint = &pathPoints[i];
        std::cout << "(" << pathPoint[0] << ", " << pathPoint[1] << ", " << pathPoint[2] << ")" << std::endl;
    }

    //pathPoints[0] = -24.8333f;
    //pathPoints[1] = 29.9167f;
    //pathPoints[2] = 24.8333f;
    //pathPoints[3] = 24.8333f;
    //pathPoints[4] = 0.25f;
    //pathPoints[5] = -24.8333f;
     
     
    //pathPoints[0] = -8;
    //pathPoints[1] = 24.1963;
    //pathPoints[2] = 15;
    //pathPoints[3] = 8;
    //pathPoints[4] = 20.0592;
    //pathPoints[5] = 8;
    //foundPathPointsCount = 2;
    _WriteToObjectFile(changeset.addedTiles, pathPoints, foundPathPointsCount);

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

