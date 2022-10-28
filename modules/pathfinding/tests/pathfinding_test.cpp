
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


void _WriteToObjectFile(
    pathfinding::NavigationMesh& navigationMesh, 
    pathfinding::NavigationMeshChangedChunk* changedChunks, 
    uint32_t changedChunksCount, 
    const float* pathPoints, 
    uint32_t numPathPoints
)
{
    std::ofstream objFile;
    objFile.open("navigation_mesh_generation.obj", std::ios::out);

    for (uint32_t i = 0; i < changedChunksCount; ++i) {
        if (changedChunks[i].changeType == 1) {
            float origin[3];
            uint16_t* triangles;
            uint32_t trianglesCount;
            float* vertices;
            uint32_t verticesCount;
            navigationMesh.getNavigationMeshDataForChunk(changedChunks[i].coords[0], changedChunks[i].coords[1], changedChunks[i].coords[2], origin, triangles, trianglesCount, vertices, verticesCount);
            objFile << "\n# tile: ("
                << changedChunks[i].coords[0] << ", "
                << changedChunks[i].coords[1] << ", "
                << changedChunks[i].coords[2] << ")"
                << std::endl;
            for (int j = 0; j < verticesCount * 3; j += 3) {
                objFile << "v "
                    << origin[0] + vertices[j] << " "
                    << origin[1] + vertices[j + 1] << " "
                    << origin[2] + vertices[j + 2]
                    << std::endl;
            }
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
    for (uint32_t i = 0; i < changedChunksCount; ++i) {
        if (changedChunks[i].changeType == 1) {
            float origin[3];
            uint16_t* triangles;
            uint32_t trianglesCount;
            float* vertices;
            uint32_t verticesCount;
            navigationMesh.getNavigationMeshDataForChunk(changedChunks[i].coords[0], changedChunks[i].coords[1], changedChunks[i].coords[2], origin, triangles, trianglesCount, vertices, verticesCount);
            for (int i = 0; i < trianglesCount * 3; i += 3) {
                objFile << "f "
                    << triangleIndicesOffset + triangles[i] + 1 << " "
                    << triangleIndicesOffset + triangles[i + 1] + 1 << " "
                    << triangleIndicesOffset + triangles[i + 2] + 1
                    << std::endl;
            }
            triangleIndicesOffset += verticesCount;
        }
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

void _PrintNavigationMeshRegenerationChanges(pathfinding::NavigationMeshChangedChunk* changedChunks, uint32_t changedChunksCount) {
    for (uint32_t i = 0; i < changedChunksCount; ++i) {
        switch (changedChunks[i].changeType) {
        case 1:
            std::cout << "Added tile at: ";
            break;
        case 2:
            std::cout << "Modified tile at: ";
            break;
        case 3:
            std::cout << "Removed tile at: ";
            break;
        }

        std::cout << "("
            << changedChunks[i].coords[0] << " ,"
            << changedChunks[i].coords[1] << " ,"
            << changedChunks[i].coords[2] << ")" << std::endl << std::endl;
    }
}

TEST(pathfinding_test_suite, creation_test)
{
    pathfinding::NavigationMesh navigationMesh;

    float chunkSize;
    navigationMesh.initialize(60.0f, 0.5f, 1.5f, 0.5f, chunkSize);
    std::cout << "chunk size: " << chunkSize << std::endl;
    
    float transform[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 2, 0, 1,
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
        25, 10, 25,
        -25, 10, 25,
        -25, -10, -25,
        25, -10, -25
    };
    unsigned short indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    pathfinding::NavigationMeshGeometryEntityHandle handle;
    navigationMesh.registerNavigationMeshGeometryEntity(transform, indices, 2, vertices, 4, handle);
    std::cout << "Registered geometry entity with handle: " << handle << std::endl;

    pathfinding::NavigationMeshChangedChunk* changedChunks; uint32_t changedChunksCount;
    navigationMesh.regenerateIfNeeded(changedChunks, changedChunksCount);

    std::cout << "Finished regenerating!" << std::endl;
    _PrintNavigationMeshRegenerationChanges(changedChunks, changedChunksCount);

    float fromPoint[3] = { 18, 0, 15 };
    //float toPoint[3] = { 8, 0, 8 };
    float toPoint[3] = { -18, 0, -15 };
    float pathPoints[300];
    uint32_t foundPathPointsCount;
    navigationMesh.findPath(fromPoint, toPoint, 100, pathPoints, foundPathPointsCount);
    std::cout << "Found path!" << std::endl;
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
     
     
    //pathPoints[0] = 18;
    //pathPoints[1] = 24.1963;
    //pathPoints[2] = 15;
    //pathPoints[3] = 8;
    //pathPoints[4] = 20.0592;
    //pathPoints[5] = 8;
    //foundPathPointsCount = 2;
    
    _WriteToObjectFile(navigationMesh, changedChunks, changedChunksCount, pathPoints, foundPathPointsCount);
}

