#pragma once

#include <stdint.h>

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Recast/Include/Recast.h>
#include <Detour/Include/DetourNavMesh.h>
#include <Detour/Include/DetourNavMeshQuery.h>

#include "Quadtree.h"

static const int32_t kTileSizeVoxels = 128; // A reasonable tile size.
static const int32_t kMaxVertsPerPoly = 3; // Ensure standard triangular navigation mesh.

// The values below were chosen through experimentation and what produced 'nice looking'
// navigation meshes.
static const int32_t kMinRegionSize = 8;
static const int32_t kMergeRegionSize = 20;
static const float kDetailSampleDistance = 6.0f;
static const float kDetailSampleMaxError = 1.0f;

// This value is recommended by the author of Recast.
static const float kMaxSimplificationError = 1.3f;

namespace pathfinding {
	enum class Result {
		kOk = 0,
		kErrorKeyNotFound = -1,
		kErrorOutOfMemory = -2,
		kErrorUnexpected = -3,
	};

	struct NavigationMeshTileData {
		int32_t tileCoordinates[2];
		float origin[3];
		uint16_t trianglesCount;
		uint16_t* triangles;
		uint16_t verticesCount;
		float* vertices;
	};

	struct NavigationMeshRegenerationChangeset {
		std::vector<NavigationMeshTileData> addedTiles;
		std::vector<NavigationMeshTileData> modifiedTiles;
		std::vector<NavigationMeshTileData> removedTiles;
	};

	using NavigationMeshGeometryEntityHandle = uint64_t;

	class NavigationMesh {
	public:
		NavigationMesh();
		~NavigationMesh();

		Result initialize(float maxWalkableSlope,
			float agentRadius,
			float agentHeight,
			float agentMaxClimb,
			const float* navMeshBoundsMin,
			const float* navMeshBoundsMax);

		Result initialize(float maxWalkableSlope, float agentRadius, float agentHeight, float agentMaxClimb);

		static const char* reflectedName;

		// TODO: Add flags & area id to be assigned to this geometry entity.
		/**
		 * @brief Creates a navigation geometry entity that will be rasterized on the navigation mesh after the next
		 *        time that 'regenerateIfNeeded' is called.
		 * @param transform The transform of the geometry entity in the navigation mesh's local space. It is used
		 * to transform the geometry entity's vertices.
		 * @param triangles The indices that form triangles from the provided vertices. Every three consecutive indices
		 * forms a triangle.
		 * @param trianglesCount The number of triangles. The size of 'triangles' is expected to be 'trianglesCount' * 3
		 * @param vertices The vertices of the geometry entity. Expected memory arrangement: x0, y0, z0, x1, y1, z1, ..., xn,
		 * yn, zn..
		 * @param verticesCount The number of vertices. The size of 'vertices' is expected to be 'verticesCount' * 3.
		 * @param handle The outputted handle to the geometry entity that was registered.
		 * @return The result code indicating success or failure.
		 */
		Result registerNavigationMeshGeometryEntity(const float* transform,
			const uint16_t* triangles,
			uint32_t trianglesCount,
			const float* vertices,
			uint32_t verticesCount,
			NavigationMeshGeometryEntityHandle& handle);

		/**
		 * @brief Removes the navigation geometry entity. It will no longer be rasterized on the navigation
		 * mesh after the next time 'regenerateIfNeeded' is called.
		 * @param handle The handle to the geometry entity that was deregistered.
		 * @return The result code indicating success or failure.
		 */
		Result deregisterNavigationMeshGeometryEntity(NavigationMeshGeometryEntityHandle handle);

		/**
		 * @brief Sets the transform of the given geometry entity in the navigation mesh's local space. This action takes
		 * effect after the next time 'regenerateIfNeeded' is called.
		 * @param handle The handle to the geometry entity that will have its transform set.
		 * @param newTransform The new transform to be assigned to the geometry entity.
		 * @return The result code indicating success or failure.
		 */
		Result setNavigationMeshGeometryEntityTransform(NavigationMeshGeometryEntityHandle handle, const float* newTransform);

		/**
		 * @brief Sets the geometry of the given geometry entity. This action takes effect after the next time
		 * 'regenerateIfNeeded' is called.
		 * @param handle The handle to the geometry entity that will have its transform set.
		 * @param newTriangles The new set of triangles to be assigned to this geometry entity.
		 * @param trianglesCount The number of triangles.
		 * @param newVertices The new set of vertices to be assigned to this geometry entity.
		 * @param verticesCount The number of vertices.
		 * @return The result code indicating success or failure.
		 */
		Result setNavigationMeshGeometryEntityGeometry(NavigationMeshGeometryEntityHandle handle,
			const uint16_t* newTriangles,
			uint32_t trianglesCount,
			const float* newVertices,
			uint32_t verticesCount);

		/**
		 * @brief Regenerate the navigation mesh tiles, if needed.
		 * @param navigationMeshRegenerationChangeset The output changeset containing all tiles that were
		 * added/modified/removed as a result of the regeneration.
		 * @return The result code indicating success or failure.
		 */
		Result regenerateIfNeeded(NavigationMeshRegenerationChangeset& navigationMeshRegenerationChangeset);

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
		Result findPath(const float* fromPoint,
			const float* toPoint,
			uint32_t maxPathPointsCount,
			float* pathPoints,
			uint32_t& foundPathPointsCount);

	private:
		enum class TileNavMeshGenStatus {
			AllocationFailure = -4,
			BuildFailure = -3,
			RasterizationFailure = -2,
			ErosionFailure = -1,
			Success = 0,
			NoGeneration = 1,
		};

		using TileKey = uint64_t;

		struct TileCoordinates {
			int32_t tx;
			int32_t ty;
		};

		struct NavigationMeshTile {
			TileCoordinates coordinates;
			dtTileRef tileRef;
			// The field below is only updated when regenerateIfNeeded is called.
			std::unordered_map<NavigationMeshGeometryEntityHandle, std::vector<uint16_t>> intersectedGeometryEntityTris;
		};

		struct Geometry {
			std::vector<uint16_t> triangles;
			std::vector<float> vertices;
		};

		struct NavigationMeshGeometryEntity {
			float transform[16];
			Geometry geometry;

			// TODO: Flags
			// TODO: Area id
			// The fields below are only updated when regenerateIfNeeded is called.
			std::vector<float> transformedGeometryVertices;
			std::vector<TileKey> intersectionCandidateTiles;
			std::vector<QuadtreeCellRef> intersectionCandidateTiles;
		};

		void cleanup();
		void processGeometryEntitySpatialChanges(NavigationMeshGeometryEntityHandle handle);
		void setGeometryEntityTransform(NavigationMeshGeometryEntity& geometryEntity, const float* transform);
		void setGeometryEntityGeometry(NavigationMeshGeometryEntity& geometryEntity,
			const uint16_t* triangles,
			uint32_t trianglesCount,
			const float* vertices,
			uint32_t verticesCount);
		bool isGeometryEntityPendingDestruction(NavigationMeshGeometryEntityHandle handle);
		NavigationMeshGeometryEntity& getGeometryEntitySafe(NavigationMeshGeometryEntityHandle handle);
		NavigationMeshGeometryEntity* findGeometryEntitySafe(NavigationMeshGeometryEntityHandle handle);
		TileCoordinates tileCoordinatesForLocalPosition(float x, float y, float z);
		TileKey keyForTileCoordinates(TileCoordinates coordinates);
		TileKey keyForTileCoordinates(int32_t tx, int32_t ty);
		NavigationMeshTile* tileForTileCoordinates(int32_t tx, int32_t ty);
		NavigationMeshTile* createTileAtCoordinates(int32_t tx, int32_t ty);
		void clearTileNavigationMesh(NavigationMeshTile* tile);
		TileNavMeshGenStatus buildTileNavigationMesh(NavigationMeshTile* tile,
			unsigned char*& navMeshData,
			int& navMeshDataSize);
		void clampAABBToNavigationMeshBounds(float* AABBMin, float* AABBMax);

		std::unordered_map<NavigationMeshGeometryEntityHandle, NavigationMeshGeometryEntity> geometryEntities_;
		// 0 is reserved for the "Invalid" handle.
		std::atomic_uint64_t nextHandle_{ 1 };

		std::unordered_map<NavigationMeshGeometryEntityHandle, NavigationMeshGeometryEntity>
			enqueuedRegisteredGeometryEntities_;
		std::unordered_set<NavigationMeshGeometryEntityHandle> enqueuedSpatiallyChangedGeometryEntities_;
		std::unordered_set<NavigationMeshGeometryEntityHandle> enqueuedDeregisteredGeometryEntities_;

		std::mutex geometryEntityRegistrationMutex_;
		std::mutex geometryEntitySpatialChangeMutex_;
		std::mutex geometryEntityDeregistrationMutex_;

		std::shared_mutex geometryEntitiesMutex_;
		std::mutex navigationMeshRegenerationMutex_;

		rcConfig tileConfig_;
		rcHeightfield* tileHeightfield_;
		rcCompactHeightfield* tileCompactHeightfield_;
		rcContourSet* tileContourSet_;
		rcPolyMesh* tilePolyMesh_;
		rcPolyMeshDetail* tilePolyMeshDetail_;
		dtNavMesh* recastNavMesh_;
		dtNavMeshQuery* recastNavQuery_;
		rcContext recastContext_;

		bool isNavMeshBounded_;
		float navMeshBoundsMin_[3];
		float navMeshBoundsMax_[3];

		float voxelSize_;
		float voxelHeight_;
		float maxWalkableSlope_;
		float agentRadius_;
		float agentHeight_;
		float agentMaxClimb_;
		float maxEdgeLen_;

		int32_t maxTiles_;
		int32_t maxPolysPerTile_;

		Quadtree<NavigationMeshTile> tileQuadtree_;
		std::unordered_map<TileKey, NavigationMeshTile> tiles_;

		struct RegenerationCandidateTile {
			NavigationMeshTile* tile;
			bool isNewlyCreated;
		};

		std::unordered_map<TileKey, RegenerationCandidateTile> regenCandidateTiles_;
	};

} // namespace pathfinding
