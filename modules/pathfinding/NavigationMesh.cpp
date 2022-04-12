#include "NavigationMesh.h"
#include <algorithm>
#include <Detour/Include/DetourNavMeshBuilder.h>

#include <iostream>

namespace pathfinding {

    /**
     * @brief For a given value, v, finds the smallest power of 2 greater than or equal to it.
     * @param v The input value.
     */
    inline uint32_t nextPow2(uint32_t v) {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }

    /**
     * @brief Finds the integer base 2 log of the given value.
     * @param v input value.
     */
    inline uint32_t ilog2(uint32_t v) {
        uint32_t r;
        uint32_t shift;
        r = (v > 0xffff) << 4;
        v >>= r;
        shift = (v > 0xff) << 3;
        v >>= shift;
        r |= shift;
        shift = (v > 0xf) << 2;
        v >>= shift;
        r |= shift;
        shift = (v > 0x3) << 1;
        v >>= shift;
        r |= shift;
        r |= (v >> 1);
        return r;
    }

    /**
     * @brief Transforms the point using the given transformation matrix.
     * @param transformMatrix4x4 The input transformation matrix.
     * @param point The input point to transform.
     * @param transformedPoint The output transformed point.
     */
    inline void TransformPoint(const float* transformMatrix4x4, const float* point, float* transformedPoint) {
        transformedPoint[0] = transformMatrix4x4[0] * point[0] + transformMatrix4x4[4] * point[1] +
            transformMatrix4x4[8] * point[2] + transformMatrix4x4[12];
        transformedPoint[1] = transformMatrix4x4[1] * point[0] + transformMatrix4x4[5] * point[1] +
            transformMatrix4x4[9] * point[2] + transformMatrix4x4[13];
        transformedPoint[2] = transformMatrix4x4[2] * point[0] + transformMatrix4x4[6] * point[1] +
            transformMatrix4x4[10] * point[2] + transformMatrix4x4[14];
    }

    /**
     * @brief Checks if two arrays of floats are approximately equal within an epsilon value.
     */
    inline bool IsApproximatelyEqual(const float* values1,
        uint32_t count1,
        const float* values2,
        uint32_t count2,
        float epsilon = 0.0001f) {
        if (count1 != count2) {
            return false;
        }

        for (std::size_t i = 0; i < count1; i++) {
            if (fabs(values1[i] - values2[i]) > epsilon) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Checks if two arrays of uint16_t integers are exactly equal.
     */
    inline bool IsEqual(const uint16_t* values1, uint32_t count1, const uint16_t* values2, uint32_t count2) {
        if (count1 != count2) {
            return false;
        }

        for (std::size_t i = 0; i < count1; i++) {
            if (values1[i] != values2[i]) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Finds the axis-aligned bounds of the given triangle
     */
    inline void calculateTriangleBounds(const float* v0,
        const float* v1,
        const float* v2,
        float* triAABBMin,
        float* triAABBMax) {
        triAABBMin[0] = std::min(std::min(v0[0], v1[0]), v2[0]);
        triAABBMin[1] = std::min(std::min(v0[1], v1[1]), v2[1]);
        triAABBMin[2] = std::min(std::min(v0[2], v1[2]), v2[2]);

        triAABBMax[0] = std::max(std::max(v0[0], v1[0]), v2[0]);
        triAABBMax[1] = std::max(std::max(v0[1], v1[1]), v2[1]);
        triAABBMax[2] = std::max(std::max(v0[2], v1[2]), v2[2]);
    }

    /**
     * @brief Obtains the navigation mesh triangulation for a single tile, which can be used to visualize the
     * navigation mesh.
     * @param polyMesh The input data structure containing the polygons for the tile's navigation mesh.
     * @param orig The origin point (in world space) of this tile's navigation mesh.
     * @param triangles The indices to values in 'vertices', where every 3 consecutive indices forms a triangle.
     * @param trianglesCount The number of triangles formed by the triangle indices. The size of 'triangles' is expected to
     * be 'trianglesCount' * 3
     * @param vertices The vertex points of this tile's navigation mesh. The points are relative to 'orig'. Expected memory
     * arrangement: x0, y0, z0, x1, y1, z1, ..., xn, yn, zn.
     * @param verticesCount The number of vertices. The size of 'vertices' is expected to be 'verticesCount' * 3.
     */
    void getTileNavigationMeshTriangulation(const rcPolyMesh& polyMesh,
        float* orig,
        uint16_t*& triangles,
        uint16_t& trianglesCount,
        float*& vertices,
        uint16_t& verticesCount) {
        const int nvp = polyMesh.nvp;
        const float cs = polyMesh.cs;
        const float ch = polyMesh.ch;
        rcVcopy(orig, polyMesh.bmin);

        trianglesCount = (nvp - 2) * polyMesh.npolys;
        triangles = new uint16_t[3 * trianglesCount];
        verticesCount = polyMesh.nverts;
        vertices = new float[3 * verticesCount];
        // TODO: area ids
        // triangleAreas = new unsigned char[trianglesCount];

        for (int i = 0; i < polyMesh.nverts; ++i) {
            const uint16_t* v = &polyMesh.verts[3 * i];
            const float x = v[0] * cs;
            const float y = (v[1] + 1) * ch;
            const float z = v[2] * cs;
            vertices[3 * i] = x;
            vertices[3 * i + 1] = y;
            vertices[3 * i + 2] = z;
        }

        for (int i = 0; i < polyMesh.npolys; ++i) {
            const uint16_t* p = &polyMesh.polys[i * nvp * 2];
            // const unsigned char area = polyMesh.areas[i];
            uint16_t vi[3];
            for (int j = 2; j < nvp; ++j) {
                if (p[j] == RC_MESH_NULL_IDX) {
                    break;
                }

                vi[0] = p[0];
                vi[1] = p[j - 1];
                vi[2] = p[j];
                for (int k = 0; k < 3; ++k) {
                    triangles[3 * ((nvp - 2) * i + (j - 2)) + k] = vi[k];
                }
            }
            // triangleAreas[i] = area;
        }
    }

    NavigationMesh::NavigationMesh() {
        tileHeightfield_ = 0;
        tileCompactHeightfield_ = 0;
        tileContourSet_ = 0;
        tilePolyMesh_ = 0;
        tilePolyMeshDetail_ = 0;
        recastNavMesh_ = 0;
        recastNavQuery_ = 0;

        isNavMeshBounded_ = false;
    }

    NavigationMesh::~NavigationMesh() {
        // Deallocate Recast stuff.
        cleanup();

        if (recastNavMesh_) {
            dtFreeNavMesh(recastNavMesh_);
            recastNavMesh_ = 0;
        }

        if (recastNavQuery_) {
            dtFreeNavMeshQuery(recastNavQuery_);
            recastNavQuery_ = 0;
        }
    }

    void NavigationMesh::cleanup() {
        if (tileHeightfield_) {
            rcFreeHeightField(tileHeightfield_);
            tileHeightfield_ = 0;
        }

        if (tileCompactHeightfield_) {
            rcFreeCompactHeightfield(tileCompactHeightfield_);
            tileCompactHeightfield_ = 0;
        }

        if (tileContourSet_) {
            rcFreeContourSet(tileContourSet_);
            tileContourSet_ = 0;
        }

        if (tilePolyMesh_) {
            rcFreePolyMesh(tilePolyMesh_);
            tilePolyMesh_ = 0;
        }

        if (tilePolyMeshDetail_) {
            rcFreePolyMeshDetail(tilePolyMeshDetail_);
            tilePolyMeshDetail_ = 0;
        }
    }

    Result NavigationMesh::initialize(float maxWalkableSlope,
        float agentRadius,
        float agentHeight,
        float agentMaxClimb,
        const float* navMeshBoundsMin,
        const float* navMeshBoundsMax) {
        isNavMeshBounded_ = true;
        rcVcopy(navMeshBoundsMin_, navMeshBoundsMin);
        rcVcopy(navMeshBoundsMax_, navMeshBoundsMax);
        return initialize(maxWalkableSlope, agentRadius, agentHeight, agentMaxClimb);
    }

    Result NavigationMesh::initialize(float maxWalkableSlope, float agentRadius, float agentHeight, float agentMaxClimb) {
        maxWalkableSlope_ = maxWalkableSlope;
        agentRadius_ = agentRadius;
        agentHeight_ = agentHeight;
        agentMaxClimb_ = agentMaxClimb;

        // According to the author of Recast, a good value for voxel size is
        // 1/2 or 1/3 of the agent radius.
        voxelSize_ = agentRadius / 3.0f;
        voxelHeight_ = voxelSize_ / 2.0f;

        // The author of recast recommends agent radius * 8 for the max edge length.
        maxEdgeLen_ = agentRadius * 8.0f;

        // There are only 22 bits available to be divided among identifying tiles
        // and identifying polys within those tiles.
        uint32_t tileBits;
        if (isNavMeshBounded_) {
            // Find the number of tiles contained in the navigation mesh bounds. We will use this value
            // to determine how many bits should be allocated for identifying tiles.
            int gridWidthVoxels;
            int gridHeightVoxels;
            rcCalcGridSize(navMeshBoundsMin_, navMeshBoundsMax_, voxelSize_, &gridWidthVoxels, &gridHeightVoxels);
            const int gridWidthTiles = (gridWidthVoxels + kTileSizeVoxels - 1) / kTileSizeVoxels;
            const int gridHeightTiles = (gridHeightVoxels + kTileSizeVoxels - 1) / kTileSizeVoxels;
            const uint32_t expectedNumTiles = gridWidthTiles * gridHeightTiles;

            // Finds the number of bits required to identify expectedNumTiles different tiles.
            // The number of bits is clamped to the range [6, 14].
            tileBits = std::min<uint32_t>(std::max<uint32_t>(ilog2(nextPow2(expectedNumTiles)), 6), 14);
        }
        else {
            tileBits = 12;
        }

        const uint32_t polyBits = 22 - tileBits;
        maxTiles_ = 1 << tileBits;
        maxPolysPerTile_ = 1 << polyBits;

        dtFreeNavMesh(recastNavMesh_);
        recastNavMesh_ = dtAllocNavMesh();
        if (!recastNavMesh_) {
            recastContext_.log(RC_LOG_ERROR, "buildTiledNavigation: Failed to allocate navmesh.");
            return Result::kErrorOutOfMemory;
        }

        // Set the navigation mesh parameters.
        dtNavMeshParams navMeshParams;
        navMeshParams.orig[0] = 0;
        navMeshParams.orig[1] = 0;
        navMeshParams.orig[2] = 0;
        navMeshParams.tileWidth = kTileSizeVoxels * voxelSize_;
        navMeshParams.tileHeight = kTileSizeVoxels * voxelSize_;
        navMeshParams.maxTiles = maxTiles_;
        navMeshParams.maxPolys = maxPolysPerTile_;

        dtStatus status;

        // Initialize the navigation mesh. This will contain the generated navigation mesh.
        status = recastNavMesh_->init(&navMeshParams);
        if (dtStatusFailed(status)) {
            recastContext_.log(RC_LOG_ERROR, "buildTiledNavigation: Failed to init navmesh.");
            return Result::kErrorUnexpected;
        }

        // Initialize the navigation mesh query. This is used for the actual pathfinding.
        recastNavQuery_ = dtAllocNavMeshQuery();
        status = recastNavQuery_->init(recastNavMesh_, 2048);
        if (dtStatusFailed(status)) {
            recastContext_.log(RC_LOG_ERROR, "buildTiledNavigation: Failed to init Detour navmesh query");
            return Result::kErrorUnexpected;
        }

        // Init build configuration used for all tiles. These fields do not change after initialization.
        memset(&tileConfig_, 0, sizeof(tileConfig_));
        tileConfig_.cs = voxelSize_;   // Voxel along x and z axis.
        tileConfig_.ch = voxelHeight_; // Voxel height (y axis).
        tileConfig_.walkableSlopeAngle = maxWalkableSlope_;
        tileConfig_.walkableHeight = (int)ceilf(agentHeight_ / tileConfig_.ch);   // Voxel coordinates.
        tileConfig_.walkableClimb = (int)floorf(agentMaxClimb_ / tileConfig_.ch); // Voxel coordinates.
        tileConfig_.walkableRadius = (int)ceilf(agentRadius_ / tileConfig_.cs);   // Voxel coordinates.
        tileConfig_.maxEdgeLen = (int)(maxEdgeLen_ / tileConfig_.cs);             // Voxel coordinates.
        tileConfig_.maxSimplificationError = kMaxSimplificationError;
        tileConfig_.minRegionArea = (int)rcSqr(kMinRegionSize);     // Note: area = size*size
        tileConfig_.mergeRegionArea = (int)rcSqr(kMergeRegionSize); // Note: area = size*size
        tileConfig_.maxVertsPerPoly = (int)kMaxVertsPerPoly;
        tileConfig_.tileSize = (int)kTileSizeVoxels;
        tileConfig_.borderSize =
            tileConfig_.walkableRadius + 3; // The 3 adds a little extra buffer to the tile border size for better results.
        tileConfig_.width = tileConfig_.tileSize + tileConfig_.borderSize * 2;
        tileConfig_.height = tileConfig_.tileSize + tileConfig_.borderSize * 2;
        tileConfig_.detailSampleDist = kDetailSampleDistance < 0.9f ? 0 : tileConfig_.cs * kDetailSampleDistance;
        tileConfig_.detailSampleMaxError = tileConfig_.ch * kDetailSampleMaxError;

        return Result::kOk;
    }

    Result NavigationMesh::registerNavigationMeshGeometryEntity(const float* transform,
        const uint16_t* triangles,
        uint32_t trianglesCount,
        const float* vertices,
        uint32_t verticesCount,
        NavigationMeshGeometryEntityHandle& handle) {
        const NavigationMeshGeometryEntityHandle geometryEntityHandle = nextHandle_.fetch_add(1, std::memory_order_relaxed);

        {
            const std::lock_guard<std::mutex> regLock(geometryEntityRegistrationMutex_);
            std::pair<std::unordered_map<NavigationMeshGeometryEntityHandle, NavigationMeshGeometryEntity>::iterator, bool>
                insertion = enqueuedRegisteredGeometryEntities_.insert({ geometryEntityHandle, {} });
            NavigationMeshGeometryEntity& geometryEntity = insertion.first->second;

            // Set transform
            setGeometryEntityTransform(geometryEntity, transform);

            // Set geometry data
            setGeometryEntityGeometry(geometryEntity, triangles, trianglesCount, vertices, verticesCount);
        }

        // Set handle to the registered geometry entity's handle.
        handle = geometryEntityHandle;

        return Result::kOk;
    }

    Result NavigationMesh::deregisterNavigationMeshGeometryEntity(NavigationMeshGeometryEntityHandle handle) {
        if (isGeometryEntityPendingDestruction(handle)) {
            // This geometry entity is already pending destruction.
            return Result::kOk;
        }

        {
            const std::lock_guard<std::mutex> regLock(geometryEntityRegistrationMutex_);
            if (enqueuedRegisteredGeometryEntities_.find(handle) != enqueuedRegisteredGeometryEntities_.end()) {
                enqueuedRegisteredGeometryEntities_.erase(handle);
                return Result::kOk;
            }
        }

        {
            const std::lock_guard<std::mutex> deregLock(geometryEntityDeregistrationMutex_);
            if (findGeometryEntitySafe(handle)) {
                // Remove pending changes to this geometry entity, if there are any.
                {
                    const std::lock_guard<std::mutex> lock(geometryEntitySpatialChangeMutex_);
                    enqueuedSpatiallyChangedGeometryEntities_.erase(handle);
                }

                enqueuedDeregisteredGeometryEntities_.insert(handle);
                return Result::kOk;
            }
        }

        return Result::kErrorKeyNotFound;
    }

    Result NavigationMesh::setNavigationMeshGeometryEntityTransform(NavigationMeshGeometryEntityHandle handle,
        const float* newTransform) {
        if (isGeometryEntityPendingDestruction(handle)) {
            // This geometry entity is queued for destruction. Do not bother modifying its data.
            return Result::kOk;
        }

        {
            const std::lock_guard<std::mutex> regLock(geometryEntityRegistrationMutex_);
            // Here we consider the case of modifying the transform of an entity
            // that is still "enqueued".
            std::unordered_map<NavigationMeshGeometryEntityHandle, NavigationMeshGeometryEntity>::iterator
                enqueuedRegisteredGeometryEntitiesIter = enqueuedRegisteredGeometryEntities_.find(handle);
            if (enqueuedRegisteredGeometryEntitiesIter != enqueuedRegisteredGeometryEntities_.end()) {
                NavigationMeshGeometryEntity& geometryEntity = enqueuedRegisteredGeometryEntitiesIter->second;
                setGeometryEntityTransform(geometryEntity, newTransform);
                return Result::kOk;
            }
        }

        {
            const std::lock_guard<std::mutex> lock(geometryEntitySpatialChangeMutex_);
            // Here we consider the case of modifying the transform of an "active"
            // entity.
            NavigationMeshGeometryEntity* geometryEntity = findGeometryEntitySafe(handle);
            if (geometryEntity) {
                if (!IsApproximatelyEqual(geometryEntity->transform, 16, newTransform, 16)) {
                    setGeometryEntityTransform(*geometryEntity, newTransform);
                    enqueuedSpatiallyChangedGeometryEntities_.insert(handle);
                }
                return Result::kOk;
            }
        }

        return Result::kErrorKeyNotFound;
    }

    Result NavigationMesh::setNavigationMeshGeometryEntityGeometry(NavigationMeshGeometryEntityHandle handle,
        const uint16_t* newTriangles,
        uint32_t newTrianglesCount,
        const float* newVertices,
        uint32_t newVerticesCount) {
        if (isGeometryEntityPendingDestruction(handle)) {
            // This geometry entity is queued for destruction. Do not bother modifying its data.
            return Result::kOk;
        }

        {
            const std::lock_guard<std::mutex> regLock(geometryEntityRegistrationMutex_);
            // Here we consider the case of modifying the geometry of an entity
            // that is still "enqueued".
            std::unordered_map<NavigationMeshGeometryEntityHandle, NavigationMeshGeometryEntity>::iterator
                enqueuedRegisteredGeometryEntitiesIter = enqueuedRegisteredGeometryEntities_.find(handle);
            if (enqueuedRegisteredGeometryEntitiesIter != enqueuedRegisteredGeometryEntities_.end()) {
                NavigationMeshGeometryEntity& geometryEntity = enqueuedRegisteredGeometryEntitiesIter->second;
                setGeometryEntityGeometry(geometryEntity, newTriangles, newTrianglesCount, newVertices, newVerticesCount);
                return Result::kOk;
            }
        }

        {
            const std::lock_guard<std::mutex> lock(geometryEntitySpatialChangeMutex_);
            // Here we consider the case of modifying the geometry of an "active"
            // entity.
            NavigationMeshGeometryEntity* geometryEntity = findGeometryEntitySafe(handle);
            if (geometryEntity) {
                if (!IsEqual(geometryEntity->geometry.triangles.data(),
                    geometryEntity->geometry.triangles.size(),
                    newTriangles,
                    newTrianglesCount) ||
                    !IsApproximatelyEqual(geometryEntity->geometry.vertices.data(),
                        geometryEntity->geometry.vertices.size(),
                        newVertices,
                        newVerticesCount)) {
                    setGeometryEntityGeometry(*geometryEntity, newTriangles, newTrianglesCount, newVertices, newVerticesCount);
                    enqueuedSpatiallyChangedGeometryEntities_.insert(handle);
                }
                return Result::kOk;
            }
        }

        return Result::kErrorKeyNotFound;
    }

    Result NavigationMesh::regenerateIfNeeded(NavigationMeshRegenerationChangeset& navigationMeshRegenerationChangeset) {
        // One navigation mesh regeneration at a time.
        const std::lock_guard<std::mutex> regenLock(navigationMeshRegenerationMutex_);

        // Phase 1: Process registered geometry entities
        {
            const std::lock_guard<std::mutex> regLock(geometryEntityRegistrationMutex_);

            for (auto iter = enqueuedRegisteredGeometryEntities_.begin(); iter != enqueuedRegisteredGeometryEntities_.end();
                ++iter) {
                const NavigationMeshGeometryEntityHandle geometryEntityHandle = iter->first;
                NavigationMeshGeometryEntity& geometryEntity = iter->second;
                {
                    const std::unique_lock<std::shared_mutex> writeLock(geometryEntitiesMutex_);
                    geometryEntities_.insert({ geometryEntityHandle, geometryEntity });
                }

                {
                    const std::lock_guard<std::mutex> lock(geometryEntitySpatialChangeMutex_);
                    enqueuedSpatiallyChangedGeometryEntities_.insert(geometryEntityHandle);
                }
            }

            enqueuedRegisteredGeometryEntities_.clear();
        }

        // Phase 2: Process deregistered geometry entities
        {
            const std::lock_guard<std::mutex> deregLock(geometryEntityDeregistrationMutex_);

            for (auto iter = enqueuedDeregisteredGeometryEntities_.begin(); iter != enqueuedDeregisteredGeometryEntities_.end();
                ++iter) {
                const NavigationMeshGeometryEntityHandle handle = *iter;

                NavigationMeshGeometryEntity& geometryEntity = getGeometryEntitySafe(handle);
                std::vector<TileKey>& previouslyIntersectedTiles = geometryEntity.intersectionCandidateTiles;

                // For each tile previously intersected by this geometry entity, remove this geometry entity's
                // handle from its dictionary of intersections, and consider the tile "dirty".
                for (TileKey tileKey : previouslyIntersectedTiles) {
                    std::unordered_map<TileKey, NavigationMeshTile>::iterator tileIter = tiles_.find(tileKey);
                    if (tileIter != tiles_.end()) {
                        NavigationMeshTile& tile = tileIter->second;
                        tile.intersectedGeometryEntityTris.erase(handle);

                        if (regenCandidateTiles_.find(tileKey) == regenCandidateTiles_.end()) {
                            regenCandidateTiles_[tileKey] = { &tile, false };
                        }
                    }
                }

                {
                    const std::unique_lock<std::shared_mutex> lock(geometryEntitiesMutex_);
                    // Delete geometry entity.
                    geometryEntities_.erase(handle);
                }
            }

            enqueuedDeregisteredGeometryEntities_.clear();
        }

        // Phase 3: Process geometry entity transform and/or geometry changes.
        {
            const std::lock_guard<std::mutex> lock(geometryEntitySpatialChangeMutex_);

            for (auto iter = enqueuedSpatiallyChangedGeometryEntities_.begin();
                iter != enqueuedSpatiallyChangedGeometryEntities_.end();
                ++iter) {
                processGeometryEntitySpatialChanges(*iter);
            }

            enqueuedSpatiallyChangedGeometryEntities_.clear();
        }

        // TODO: Phase 4: Process geometry entity flag/area changes.

        // Phase 5: Iterate candidate tiles and perform navigation mesh generation if necessary.

        for (std::unordered_map<TileKey, RegenerationCandidateTile>::iterator regenCandidateTileIter =
            regenCandidateTiles_.begin();
            regenCandidateTileIter != regenCandidateTiles_.end();
            ++regenCandidateTileIter) {
            const TileKey regenCandidateTileKey = regenCandidateTileIter->first;
            const bool isTileNewlyCreated = regenCandidateTileIter->second.isNewlyCreated;
            NavigationMeshTile* regenCandidateTile = regenCandidateTileIter->second.tile;

            if (!regenCandidateTile->intersectedGeometryEntityTris.empty()) {
                // This tile potentially intersects with geometry entities.
                int navmeshDataSize = 0;
                unsigned char* navmeshData;
                TileNavMeshGenStatus genStatus = buildTileNavigationMesh(regenCandidateTile, navmeshData, navmeshDataSize);

                switch (genStatus) {
                case TileNavMeshGenStatus::Success: {
                    // Clear old navigation mesh for this tile.
                    clearTileNavigationMesh(regenCandidateTile);

                    // Add newly built navigation mesh for this tile.
                    dtStatus status = recastNavMesh_->addTile(navmeshData, navmeshDataSize, DT_TILE_FREE_DATA, 0, 0);
                    if (dtStatusFailed(status)) {
                        // Free the navmesh data because we failed to add the tile's navigation mesh.
                        dtFree(navmeshData);
                    }
                    else {
                        // The tile's navigation mesh was successfully incorporated into the main navigation mesh.
                        //
                        // Fetch data for this tile's generated navigation mesh.
                        NavigationMeshTileData navMeshTileData;
                        navMeshTileData.tileCoordinates[0] = regenCandidateTile->coordinates.tx;
                        navMeshTileData.tileCoordinates[1] = regenCandidateTile->coordinates.ty;
                        getTileNavigationMeshTriangulation(*tilePolyMesh_,
                            navMeshTileData.origin,
                            navMeshTileData.triangles,
                            navMeshTileData.trianglesCount,
                            navMeshTileData.vertices,
                            navMeshTileData.verticesCount);

                        if (isTileNewlyCreated) {
                            navigationMeshRegenerationChangeset.addedTiles.push_back(navMeshTileData);
                        }
                        else {
                            navigationMeshRegenerationChangeset.modifiedTiles.push_back(navMeshTileData);
                        }
                    }
                } break;
                case TileNavMeshGenStatus::NoGeneration: {
                    // No navigation mesh was generated for this tile. It is a false positive.
                    if (isTileNewlyCreated) {
                        // If tile was newly created, delete it.
                        tiles_.erase(regenCandidateTileKey);
                    }
                } break;
                default:
                    // no-op. There was straight up an issue with this tile's navmesh generation.
                    break;
                }
            }
            else {
                // This tile has no geometry intersecting it. Delete it.
                // TODO: Assert that isTileNewlyCreated is false.
                NavigationMeshTileData navMeshTileData;
                navMeshTileData.tileCoordinates[0] = regenCandidateTile->coordinates.tx;
                navMeshTileData.tileCoordinates[1] = regenCandidateTile->coordinates.ty;
                navigationMeshRegenerationChangeset.removedTiles.push_back(navMeshTileData);
                clearTileNavigationMesh(regenCandidateTile);
                tiles_.erase(regenCandidateTileKey);
            }
        }

        regenCandidateTiles_.clear();

        return Result::kOk;
    }

    void NavigationMesh::processGeometryEntitySpatialChanges(NavigationMeshGeometryEntityHandle handle) {
        NavigationMeshGeometryEntity& geometryEntity = getGeometryEntitySafe(handle);

        // Find tiles that were previously intersected by this geometry entity. They are likely
        // affected by the spatial changes applied to this geometry entity.
        for (TileKey tileKey : geometryEntity.intersectionCandidateTiles) {
            std::unordered_map<TileKey, NavigationMeshTile>::iterator tileIter = tiles_.find(tileKey);
            if (tileIter != tiles_.end()) {
                NavigationMeshTile& tile = tileIter->second;
                // Assume that this geometry entity no longer intersects this tile.
                // If we are wrong, it will be corrected in the next step anyway, where
                // we check for new geometry entity-tile intersections after the vertices
                // are transformed.
                tile.intersectedGeometryEntityTris.erase(handle);

                if (regenCandidateTiles_.find(tileKey) == regenCandidateTiles_.end()) {
                    regenCandidateTiles_[tileKey] = { &tile, false };
                }
            }
        }

        // Clear previously intersected tiles. We will calculate new ones.
        geometryEntity.intersectionCandidateTiles.clear();

        // Transform geometry verts to navigation mesh local space using new transform and/or new geometry.
        geometryEntity.transformedGeometryVertices.clear();
        for (std::size_t i = 0; i < geometryEntity.geometry.vertices.size(); i += 3) {
            float vertex[3] = { geometryEntity.geometry.vertices[i],
                               geometryEntity.geometry.vertices[i + 1],
                               geometryEntity.geometry.vertices[i + 2] };
            float transformedVertex[3];
            TransformPoint(geometryEntity.transform, vertex, transformedVertex);
            geometryEntity.transformedGeometryVertices.push_back(transformedVertex[0]); // push_back x
            geometryEntity.transformedGeometryVertices.push_back(transformedVertex[1]); // push_back y
            geometryEntity.transformedGeometryVertices.push_back(transformedVertex[2]); // push_back z
        }

        // Find tiles affected by the geometry entity after its geometry is transformed.
        for (std::size_t triIndex = 0; triIndex < geometryEntity.geometry.triangles.size(); triIndex += 3) {
            const uint16_t v0i = geometryEntity.geometry.triangles[triIndex];
            const uint16_t v1i = geometryEntity.geometry.triangles[triIndex + 1];
            const uint16_t v2i = geometryEntity.geometry.triangles[triIndex + 2];

            const float* v0 = &geometryEntity.transformedGeometryVertices[3 * v0i];
            const float* v1 = &geometryEntity.transformedGeometryVertices[3 * v1i];
            const float* v2 = &geometryEntity.transformedGeometryVertices[3 * v2i];

            // Calculate AABB for triangle.
            float triAABBMin[3];
            float triAABBMax[3];
            calculateTriangleBounds(v0, v1, v2, triAABBMin, triAABBMax);

            // Expand triangle AABB by the tile border.
            // It is faster to expand the triangle AABB by the border size and check for intersections
            // with surrounding tiles than it is to expand each tile AABB by the border size and check if
            // it intersects this triangle.
            const float bs = tileConfig_.borderSize * tileConfig_.cs;
            triAABBMin[0] -= bs;
            triAABBMin[2] -= bs;
            triAABBMax[0] += bs;
            triAABBMax[2] += bs;

            if (isNavMeshBounded_) {
                if (triAABBMax[0] <= navMeshBoundsMin_[0] || triAABBMin[0] >= navMeshBoundsMax_[0] ||
                    triAABBMax[1] <= navMeshBoundsMin_[1] || triAABBMin[1] >= navMeshBoundsMax_[1] ||
                    triAABBMax[2] <= navMeshBoundsMin_[2] || triAABBMin[2] >= navMeshBoundsMax_[2]) {
                    // The triangle AABB is entirely outside of the navigation mesh bounds. Skip.
                    continue;
                }
                clampAABBToNavigationMeshBounds(triAABBMin, triAABBMax);
            }

            const TileCoordinates minTileCoords = tileCoordinatesForLocalPosition(triAABBMin[0], triAABBMin[1], triAABBMin[2]);
            const TileCoordinates maxTileCoords = tileCoordinatesForLocalPosition(triAABBMax[0], triAABBMax[1], triAABBMax[2]);
            // This may produce some false positives for tile-triangle intersections, but that is ok because
            // the false positives are later filtered out internally by Recast during the rasterization process.
            for (int32_t tx = minTileCoords.tx; tx <= maxTileCoords.tx; tx++) {
                for (int32_t ty = minTileCoords.ty; ty <= maxTileCoords.ty; ty++) {
                    const TileKey tileKey = keyForTileCoordinates(tx, ty);
                    NavigationMeshTile* tile = tileForTileCoordinates(tx, ty);
                    if (!tile) {
                        // Create a new tile.
                        tile = createTileAtCoordinates(tx, ty);
                        regenCandidateTiles_[tileKey] = { tile, true };
                    }
                    else if (regenCandidateTiles_.find(tileKey) == regenCandidateTiles_.end()) {
                        regenCandidateTiles_[tileKey] = { tile, false };
                    }

                    auto geometryEntityTrisIter = tile->intersectedGeometryEntityTris.find(handle);
                    if (geometryEntityTrisIter != tile->intersectedGeometryEntityTris.end()) {
                        geometryEntityTrisIter->second.push_back(v0i);
                        geometryEntityTrisIter->second.push_back(v1i);
                        geometryEntityTrisIter->second.push_back(v2i);
                    }
                    else {
                        tile->intersectedGeometryEntityTris[handle] = { v0i, v1i, v2i };
                    }

                    geometryEntity.intersectionCandidateTiles.push_back(tileKey);
                }
            }
        }
    }

    void NavigationMesh::setGeometryEntityTransform(NavigationMesh::NavigationMeshGeometryEntity& geometryEntity,
        const float* transform) {
        memcpy(geometryEntity.transform, transform, 16 * sizeof(float));
    }

    void NavigationMesh::setGeometryEntityGeometry(NavigationMesh::NavigationMeshGeometryEntity& geometryEntity,
        const uint16_t* triangles,
        uint32_t trianglesCount,
        const float* vertices,
        uint32_t verticesCount) {
        geometryEntity.geometry.triangles.clear();
        geometryEntity.geometry.triangles.insert(geometryEntity.geometry.triangles.begin(),
            triangles,
            triangles + 3 * trianglesCount);
        geometryEntity.geometry.vertices.clear();
        geometryEntity.geometry.vertices.insert(geometryEntity.geometry.vertices.begin(),
            vertices,
            vertices + 3 * verticesCount);
    }

    bool NavigationMesh::isGeometryEntityPendingDestruction(NavigationMeshGeometryEntityHandle handle) {
        const std::lock_guard<std::mutex> lock(geometryEntityDeregistrationMutex_);
        return enqueuedDeregisteredGeometryEntities_.find(handle) != enqueuedDeregisteredGeometryEntities_.end();
    }

    NavigationMesh::NavigationMeshGeometryEntity& NavigationMesh::getGeometryEntitySafe(
        NavigationMeshGeometryEntityHandle handle) {
        const std::shared_lock<std::shared_mutex> lock(geometryEntitiesMutex_);
        return geometryEntities_[handle];
    }

    NavigationMesh::NavigationMeshGeometryEntity* NavigationMesh::findGeometryEntitySafe(
        NavigationMeshGeometryEntityHandle handle) {
        const std::shared_lock<std::shared_mutex> lock(geometryEntitiesMutex_);
        std::unordered_map<NavigationMeshGeometryEntityHandle, NavigationMeshGeometryEntity>::iterator geometryEntitiesIter =
            geometryEntities_.find(handle);
        if (geometryEntitiesIter != geometryEntities_.end()) {
            return &geometryEntitiesIter->second;
        }
        else {
            return nullptr;
        }
    }

    NavigationMesh::TileCoordinates NavigationMesh::tileCoordinatesForLocalPosition(float x, float y, float z) {
        (void)y;
        const float ts = tileConfig_.tileSize * tileConfig_.cs;
        const int32_t tx = (int32_t)(floorf(x / ts));
        const int32_t ty = (int32_t)(floorf(z / ts));
        TileCoordinates coordinates = { tx, ty };
        return coordinates;
    }

    NavigationMesh::TileKey NavigationMesh::keyForTileCoordinates(TileCoordinates coordinates) {
        return keyForTileCoordinates(coordinates.tx, coordinates.ty);
    }

    NavigationMesh::TileKey NavigationMesh::keyForTileCoordinates(int32_t tx, int32_t ty) {
        TileKey tileKey;
        char* tileKeyPtr = reinterpret_cast<char*>(&tileKey);
        memcpy(tileKeyPtr, &tx, sizeof(tx));
        memcpy(tileKeyPtr + sizeof(tx), &ty, sizeof(ty));
        return tileKey;
    }

    NavigationMesh::NavigationMeshTile* NavigationMesh::tileForTileCoordinates(int32_t tx, int32_t ty) {
        TileCoordinates coordinates = { tx, ty };
        const TileKey tileKey = keyForTileCoordinates(coordinates);
        std::unordered_map<TileKey, NavigationMeshTile>::iterator tileIter = tiles_.find(tileKey);
        if (tileIter == tiles_.end()) {
            return nullptr;
        }
        return &tileIter->second;
    }

    NavigationMesh::NavigationMeshTile* NavigationMesh::createTileAtCoordinates(int32_t tx, int32_t ty) {
        TileCoordinates coordinates = { tx, ty };
        const TileKey tileKey = keyForTileCoordinates(coordinates);
        std::pair<std::unordered_map<TileKey, NavigationMeshTile>::iterator, bool> insertion =
            tiles_.insert({ tileKey, {coordinates, {}} });
        return &(insertion.first->second);
    }

    void NavigationMesh::clearTileNavigationMesh(NavigationMeshTile* tile) {
        recastNavMesh_->removeTile(recastNavMesh_->getTileRefAt(tile->coordinates.tx, tile->coordinates.ty, 0), 0, 0);
    }

    NavigationMesh::TileNavMeshGenStatus NavigationMesh::buildTileNavigationMesh(NavigationMeshTile* tile,
        unsigned char*& navMeshData,
        int& navMeshDataSize) {
        // Find axis aligned bounding box of the tile.
        const float ts = tileConfig_.tileSize * tileConfig_.cs;
        const float minX = tile->coordinates.tx * ts;
        const float minZ = tile->coordinates.ty * ts;

        float tileAABBMin[3] = { minX, 0, minZ };
        float tileAABBMax[3] = { minX + ts, 0, minZ + ts };
        if (isNavMeshBounded_) {
            tileAABBMin[1] = navMeshBoundsMin_[1];
            tileAABBMax[1] = navMeshBoundsMax_[1];
            clampAABBToNavigationMeshBounds(tileAABBMin, tileAABBMax);
        }
        else {
            // This navigation mesh is unbounded, but we should not set the tile min/max Y values
            // to large neg/pos values because it would introduce significant floating-point
            // precision errors in Recast's internal calculations. Instead, we set the min/max
            // Y values to those of the intersected triangle vertices.
            for (auto& intersectedGeometryEntityTris : tile->intersectedGeometryEntityTris) {
                const NavigationMeshGeometryEntity& geometryEntity = geometryEntities_[intersectedGeometryEntityTris.first];
                const std::vector<uint16_t>& intersectedTriangles = intersectedGeometryEntityTris.second;
                for (uint16_t vi : intersectedTriangles) {
                    tileAABBMin[1] = std::min(tileAABBMin[1], geometryEntity.transformedGeometryVertices[3 * vi + 1]);
                    tileAABBMax[1] = std::max(tileAABBMax[1], geometryEntity.transformedGeometryVertices[3 * vi + 1]);
                }
            }
            tileAABBMax[1] += agentHeight_ + 3;
        }

        rcVcopy(tileConfig_.bmin, tileAABBMin);
        rcVcopy(tileConfig_.bmax, tileAABBMax);

        // Expand this box along xz plane by borderSize * cellSize to account for geometry near
        // the edges of the tile.
        const float bs = tileConfig_.borderSize * tileConfig_.cs;
        tileConfig_.bmin[0] -= bs;
        tileConfig_.bmin[2] -= bs;
        tileConfig_.bmax[0] += bs;
        tileConfig_.bmax[2] += bs;

        // Clean up intermediate tile pipeline stuff.
        cleanup();

        // Reset build times gathering.
        recastContext_.resetTimers();

        // Start the build process.
        recastContext_.startTimer(RC_TIMER_TOTAL);

        recastContext_.log(RC_LOG_PROGRESS, "Building tile at: (%d, %d)", tile->coordinates.tx, tile->coordinates.ty);

        // Allocate voxel heightfield where we rasterize our input data to.
        tileHeightfield_ = rcAllocHeightfield();
        if (!tileHeightfield_) {
            recastContext_.log(RC_LOG_ERROR,
                "buildNavigation: Out of memory for heightfield allocation for 'tileHeightfield_'.");
            return TileNavMeshGenStatus::AllocationFailure;
        }
        if (!rcCreateHeightfield(&recastContext_,
            *tileHeightfield_,
            tileConfig_.width,
            tileConfig_.height,
            tileConfig_.bmin,
            tileConfig_.bmax,
            tileConfig_.cs,
            tileConfig_.ch)) {
            recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to create solid heightfield.");
            return TileNavMeshGenStatus::BuildFailure;
        }

        // Rasterize all intersecting geometry entities onto the tile.
        for (auto& intersectedGeometryEntityTris : tile->intersectedGeometryEntityTris) {
            const NavigationMeshGeometryEntityHandle handle = intersectedGeometryEntityTris.first;
            const NavigationMeshGeometryEntity& geometryEntity = geometryEntities_[handle];
            const std::vector<uint16_t>& intersectedTriangles = intersectedGeometryEntityTris.second;

            // Not all of these vertices will be used. Only those indexed by intersectedTriangles will be used by Recast.
            const float* verts = geometryEntity.transformedGeometryVertices.data();
            const std::size_t nverts = geometryEntity.transformedGeometryVertices.size() / 3;
            const uint16_t* ctris = intersectedTriangles.data();
            const std::size_t nctris = intersectedTriangles.size() / 3;

            // memset(m_triareas, 0, nctris*sizeof(unsigned char));
            // rcMarkWalkableTriangles(&recastContext_, tileConfig_.walkableSlopeAngle, verts, nverts, ctris, nctris,
            // m_triareas);

            unsigned char* triareas = new unsigned char[nctris];
            memset(triareas, RC_WALKABLE_AREA, nctris * sizeof(unsigned char));

            // TODO: Set area ids of triangles for this entity
            bool success = rcRasterizeTriangles(&recastContext_,
                verts,    // Vertices
                nverts,   // Number of vertices
                ctris,    // Triangle indices
                triareas, // Area id corresponding to these triangles. TODO: Set this field to
                          // the area id of this geometry entity.
                nctris,   // Number of triangles
                *tileHeightfield_,        // Heightfield
                tileConfig_.walkableClimb // Walkable climb
            );

            delete[] triareas;

            if (!success) {
                recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to rasterize triangles.");
                return TileNavMeshGenStatus::RasterizationFailure;
            }
        }

        // Filter out unwanted overhangs caused by Recast's conservative rasterization.
        rcFilterLowHangingWalkableObstacles(&recastContext_, tileConfig_.walkableClimb, *tileHeightfield_);

        // Filter out spans where the character cannot stand.
        rcFilterLedgeSpans(&recastContext_, tileConfig_.walkableHeight, tileConfig_.walkableClimb, *tileHeightfield_);
        rcFilterWalkableLowHeightSpans(&recastContext_, tileConfig_.walkableHeight, *tileHeightfield_);

        // Compact the heightfield so that it is faster to handle from now on.
        // This will result more cache coherent data as well as the neighbours
        // between walkable cells will be calculated.
        tileCompactHeightfield_ = rcAllocCompactHeightfield();
        if (!tileCompactHeightfield_) {
            recastContext_.log(
                RC_LOG_ERROR,
                "buildNavigation: Out of memory for compact heightfield allocation for 'tileCompactHeightfield_'.");
            return TileNavMeshGenStatus::AllocationFailure;
        }
        if (!rcBuildCompactHeightfield(&recastContext_,
            tileConfig_.walkableHeight,
            tileConfig_.walkableClimb,
            *tileHeightfield_,
            *tileCompactHeightfield_)) {
            recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to build compact heightfield.");
            return TileNavMeshGenStatus::BuildFailure;
        }

        // Erode the walkable area by agent radius.
        if (!rcErodeWalkableArea(&recastContext_, tileConfig_.walkableRadius, *tileCompactHeightfield_)) {
          recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to erode.");
          return TileNavMeshGenStatus::ErosionFailure;
        }

        // Prepare for Watershed region partitioning by calculating distance field along the walkable surface.
        if (!rcBuildDistanceField(&recastContext_, *tileCompactHeightfield_)) {
            recastContext_.log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
            return TileNavMeshGenStatus::BuildFailure;
        }

        // Partition the walkable surface into simple regions without holes.
        if (!rcBuildRegions(&recastContext_,
            *tileCompactHeightfield_,
            tileConfig_.borderSize,
            tileConfig_.minRegionArea,
            tileConfig_.mergeRegionArea)) {
            recastContext_.log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
            return TileNavMeshGenStatus::BuildFailure;
        }

        // Create contours.
        tileContourSet_ = rcAllocContourSet();
        if (!tileContourSet_) {
            recastContext_.log(RC_LOG_ERROR,
                "buildNavigation: Out of memory for contour set allocation for 'tileContourSet_'.");
            return TileNavMeshGenStatus::AllocationFailure;
        }
        if (!rcBuildContours(&recastContext_,
            *tileCompactHeightfield_,
            tileConfig_.maxSimplificationError,
            tileConfig_.maxEdgeLen,
            *tileContourSet_)) {
            recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to create contours.");
            return TileNavMeshGenStatus::BuildFailure;
        }

        if (tileContourSet_->nconts == 0) {
            // Zero contours were produce. Return early because no navmesh will be generated for this tile.
            return TileNavMeshGenStatus::NoGeneration;
        }

        // Build polygon navmesh from the contours.
        tilePolyMesh_ = rcAllocPolyMesh();
        if (!tilePolyMesh_) {
            recastContext_.log(RC_LOG_ERROR, "buildNavigation: Out of memory for poly mesh allocation for 'tilePolyMesh_'.");
            return TileNavMeshGenStatus::AllocationFailure;
        }

        if (!rcBuildPolyMesh(&recastContext_, *tileContourSet_, tileConfig_.maxVertsPerPoly, *tilePolyMesh_)) {
            recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to triangulate contours.");
            return TileNavMeshGenStatus::BuildFailure;
        }

        // Build detail mesh.
        tilePolyMeshDetail_ = rcAllocPolyMeshDetail();
        if (!tilePolyMeshDetail_) {
            recastContext_.log(RC_LOG_ERROR,
                "buildNavigation: Out of memory for poly mesh detail allocation for 'tilePolyMeshDetail_'.");
            return TileNavMeshGenStatus::AllocationFailure;
        }

        if (!rcBuildPolyMeshDetail(&recastContext_,
            *tilePolyMesh_,
            *tileCompactHeightfield_,
            tileConfig_.detailSampleDist,
            tileConfig_.detailSampleMaxError,
            *tilePolyMeshDetail_)) {
            recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to build polymesh detail.");
            return TileNavMeshGenStatus::BuildFailure;
        }

        // Build the navmesh for this tile.
        dtNavMeshCreateParams params;
        memset(&params, 0, sizeof(params));
        params.verts = tilePolyMesh_->verts;
        params.vertCount = tilePolyMesh_->nverts;
        params.polys = tilePolyMesh_->polys;
        params.polyAreas = tilePolyMesh_->areas;
        params.polyFlags = tilePolyMesh_->flags;
        params.polyCount = tilePolyMesh_->npolys;
        params.nvp = tilePolyMesh_->nvp;
        params.detailMeshes = tilePolyMeshDetail_->meshes;
        params.detailVerts = tilePolyMeshDetail_->verts;
        params.detailVertsCount = tilePolyMeshDetail_->nverts;
        params.detailTris = tilePolyMeshDetail_->tris;
        params.detailTriCount = tilePolyMeshDetail_->ntris;
        // TODO: Off mesh connection parameters.
        params.walkableHeight = agentHeight_;
        params.walkableRadius = agentRadius_;
        params.walkableClimb = agentMaxClimb_;
        params.tileX = tile->coordinates.tx;
        params.tileY = tile->coordinates.ty;
        params.tileLayer = 0;
        rcVcopy(params.bmin, tilePolyMesh_->bmin);
        rcVcopy(params.bmax, tilePolyMesh_->bmax);
        params.cs = tileConfig_.cs;
        params.ch = tileConfig_.ch;
        params.buildBvTree = true;

        if (!dtCreateNavMeshData(&params, &navMeshData, &navMeshDataSize)) {
            recastContext_.log(RC_LOG_ERROR, "Failed to build Detour navmesh.");
            return TileNavMeshGenStatus::BuildFailure;
        }

        recastContext_.stopTimer(RC_TIMER_TOTAL);
        float tileBuildTime = recastContext_.getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;
        float tileMemoryUsage = navMeshDataSize / 1024.0f;

        recastContext_.log(RC_LOG_PROGRESS, ">> Tile Build Time: %f sec", tileBuildTime);
        recastContext_.log(RC_LOG_PROGRESS, ">> Tile Memory Usage: %f KB", tileMemoryUsage);
        recastContext_.log(RC_LOG_PROGRESS,
            ">> Tile polymesh has: %d vertices and %d polygons",
            tilePolyMesh_->nverts,
            tilePolyMesh_->npolys);

        return TileNavMeshGenStatus::Success;
    }

    void NavigationMesh::clampAABBToNavigationMeshBounds(float* AABBMin, float* AABBMax) {
        AABBMin[0] = std::max(AABBMin[0], navMeshBoundsMin_[0]);
        AABBMin[1] = std::max(AABBMin[1], navMeshBoundsMin_[1]);
        AABBMin[2] = std::max(AABBMin[2], navMeshBoundsMin_[2]);

        AABBMax[0] = std::min(AABBMax[0], navMeshBoundsMax_[0]);
        AABBMax[1] = std::min(AABBMax[1], navMeshBoundsMax_[1]);
        AABBMax[2] = std::min(AABBMax[2], navMeshBoundsMax_[2]);
    }

    Result NavigationMesh::findPath(const float* fromPoint,
        const float* toPoint,
        uint32_t maxPathPointsCount,
        float* pathPoints,
        uint32_t& foundPathPointsCount) {
        // TODO: Call detour methods to find path.
        (void)fromPoint;
        (void)toPoint;
        (void)maxPathPointsCount;
        foundPathPointsCount = 0;
        pathPoints = nullptr;
        return Result::kOk;
    }

} // namespace pathfinding
