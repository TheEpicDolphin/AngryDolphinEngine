/**
 * (c) Meta, Inc. and its affiliates. Confidential and proprietary.
 */
#pragma once

#include <stdint.h>

#include <atomic>
#include <cmath>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Recast/Include/Recast.h>
#include <Detour/Include/DetourNavMesh.h>
#include <Detour/Include/DetourNavMeshQuery.h>

#include <core/utils/orthtree.h>
#include "AABB.h"
#include "NavigableSurfaceRegistry.h"

static const double pi = 3.14159265f;

static const int32_t kTileSize = 128; // A reasonable tile size.
static const int32_t kMaxVertsPerPoly = 3;  // Ensure standard triangular navigation mesh.

// The values below were chosen through experimentation and what produced 'nice looking'
// navigation meshes.
static const int32_t kMinRegionArea = 64;
static const int32_t kMergeRegionArea = 400;
static const float kDetailSampleDistance = 6.0f;
static const float kDetailSampleMaxError = 1.0f;

// This value is recommended by the author of Recast.
static const float kMaxSimplificationError = 1.3f;

// The maximum number of polygons allowed in a path along the navigation mesh.
static const uint32_t kMaxPolyPathSize = 5000;

namespace pathfinding {
	enum class Result {
		kOk = 0,
		kErrorKeyNotFound = -1,
		kErrorOutOfMemory = -2,
		kErrorUnexpected = -3,
	};

	using NavigationMeshChunkRef = OrthtreeCellRef;

	struct NavigationMeshChangedChunk {
		NavigationMeshChunkRef chunkRef;
		unsigned char changeType;
	};

	struct NavigationMeshConfigFeedback {
		float validMaxWalkableSlopeRange[2];
		float validAgentRadiusRange[2];
		float validAgentHeightRange[2];
		float validAgentMaxClimbRange[2];
		float validSizeRanges[3][2];
		float size[3];
		const char errorMessage[50];
	};

	class NavigationMesh {
	public:
		NavigationMesh(std::shared_ptr<NavigableSurfaceRegistry> navSurfaceRegistry);
		~NavigationMesh();

		Result configure(
			float maxWalkableSlope,
			float agentRadius,
			float agentHeight,
			float agentMaxClimb,
			const float* targetSize,
			NavigationMeshConfigFeedback& feedback
		);

		Result setWorld2LocalTransform(const float& transformMatrix4x4);

		/**
		 * @brief Regenerate the navigation mesh tiles, if needed.
		 * @param changedTiles The output changeset containing all tiles that were
		 * added/modified/removed as a result of the regeneration.
		 * @param changedTilesCount The number of tiles in changedTiles.
		 * @return The result code indicating success or failure.
		 */
		Result regenerateIfNeeded(
			NavigationMeshChangedChunk*& changedChunks,
			uint32_t& changedChunksCount);

		/**
		 * @brief TODO
		 * @param center (in) World space.
		 * @param radius (in) TODO
		 * @param chunks (out) TODO
		 * @param chunksCount (out) TODO
		 * @return The result code indicating success or failure.
		 */
		Result getChunksIntersectingCircle(
			const float* center,
			float radius,
			NavigationMeshChunkRef* chunks,
			uint32_t chunksCount);

		/**
		 * @brief Acquires the triangles and vertices of the navigation mesh
		 * corresponding to the inputted chunk reference.
		 * @param chunkRef (in) Chunk to fetch data for.
		 * @param origin (out) The origin point of the chunk. All vertex position
		 * are relative to this point.
		 * @param triangles (out) The triangle indices of the chunk's navigation mesh.
		 * @param trianglesCount (out) The number of triangles.
		 * @param vertices (out) The vertices of the chunk's navigation mesh.
		 * @param veticesCount (out) The number of vertices.
		 * mesh.
		 * @return The result code indicating success or failure.
		 */
		Result getNavigationMeshDataForChunk(
			NavigationMeshChunkRef chunkRef,
			float* origin,
			uint16_t*& triangles,
			uint32_t& trianglesCount,
			float*& vertices,
			uint32_t& verticesCount);

		/**
		 * @brief Finds the path between two points on the navigation mesh.
		 * @param fromPoint The starting point.
		 * @param toPoint The destination point.
		 * @param maxPathPointsCount The maximum number of points to find along the path from fromPoint to toPoint.
		 * @param pathPoints An array of min(foundPathPointsCount, maxPathPointsCount) points along the path from 'fromPoint'
		 * to 'toPoint'. If no path is found, this is set to nullptr.
		 * @param foundPathPointsCount The number of points outputted in pathPoints. If 0, no path exists between the two
		 * points.
		 * @return The result code indicating success or failure.
		 */
		Result findPath(
			const float* fromPoint,
			const float* toPoint,
			uint32_t maxPathPointsCount,
			float* pathPoints,
			uint32_t& foundPathPointsCount);

	private:
		struct NavigationMeshChunk {
			dtTileRef tileRef;

			// The field below is only updated when regenerateIfNeeded is called.
			//std::unordered_map<NavigationMeshGeometryEntityHandle, std::vector<uint16_t>> intersectedGeometryEntityTris;
			std::unordered_map<NavigableSurfaceGroupRef, std::vector<NavigableSurfaceIndex>> intersectedSurfaces;
		};
		orthtree<3, NavigationMeshChunk> chunkQuatree_;

		struct NavigableSurfaceGroupState {
			float relativeToNavmeshTransform[16];
			std::vector<NavigationMeshChunkRef> potentiallyIntersectingChunks;
		};
		std::unordered_map<NavigableSurfaceGroupRef, NavigableSurfaceGroupState> groupStates_;

		std::unordered_set<NavigableSurfaceGroupRef> displacedGroups_;
		std::vector<NavigableSurfaceGroupRef> placedGroups_;

		struct RegenCandidateChunkData {
			bool isNewlyCreated;
		};
		std::unordered_map<NavigationMeshChunkRef, RegenCandidateChunkData> regenCandidateChunks_;

		enum class ChunkNavMeshGenStatus {
			AllocationFailure = -4,
			BuildFailure = -3,
			RasterizationFailure = -2,
			ErosionFailure = -1,
			Success = 0,
			NoGeneration = 1,
		};

		void cleanup();
		void processDisplacedGroup(NavigableSurfaceGroupRef groupRef);
		void processPlacedGroup(NavigableSurfaceGroupRef groupRef);
		void clearNavigationMeshForChunk(NavigationMeshChunk* chunk);
		ChunkNavMeshGenStatus buildNavigationMeshAt(
			int16_t tx,
			int16_t ty,
			int16_t tz,
			const std::unordered_map<NavigationMeshGeometryEntityHandle, std::vector<uint16_t>>& intersectedGeometryEntityTris,
			unsigned char*& navMeshData,
			int& navMeshDataSize
		);

		std::shared_ptr<NavigableSurfaceRegistry> navSurfaceRegistry_;
		AABB worldBounds_;
		float toLocalTransform_[16];
		float size_[3];
		int32_t xTilesRange[2];
		int32_t zTilesRange[2];

		std::mutex navigationMeshRegenerationMutex_;
		std::shared_mutex navMeshTileMutex_;
		std::shared_mutex navMeshQueryMutex_;

		rcConfig tileConfig_;
		rcHeightfield* tileHeightfield_;
		rcCompactHeightfield* tileCompactHeightfield_;
		rcContourSet* tileContourSet_;
		rcPolyMesh* tilePolyMesh_;
		rcPolyMeshDetail* tilePolyMeshDetail_;
		dtNavMesh* recastNavMesh_;
		dtNavMeshQuery* recastNavQuery_;
		rcContext recastContext_;

		std::vector<NavigationMeshChangedChunk> changedChunksFromLastRegeneration_;
		std::vector<uint16_t> chunkNavigationMeshTriangles_;
		std::vector<float> chunkNavigationMeshVertices_;

		std::vector<dtPolyRef> polyPath_;

		float maxWalkableSlope_;
		float agentRadius_;
		float agentHeight_;
		float agentMaxClimb_;
		float maxEdgeLen_;
	};

} // namespace pathfinding
