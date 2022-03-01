/**
 * (c) Facebook, Inc. and its affiliates. Confidential and proprietary.
 */
#pragma once

#include <stdint.h>

#include <atomic>
#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <Recast/Include/Recast.h>
#include <Detour/Include/DetourNavMesh.h>
#include <Detour/Include/DetourNavMeshQuery.h>

static const int32_t kTileSize = 32;
static const int32_t kMaxVertsPerPoly = 3;  // Ensure standard triangular navigation mesh.
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

struct NavigationMeshTileRegenerationResults {
	int32_t tx;
	int32_t ty;
	std::vector<float> navMeshVertices;
	//float duration;	
};

struct NavigationMeshRegenerationChangeset {
	std::vector<NavigationMeshTileRegenerationResults> addedTiles;
	std::vector<NavigationMeshTileRegenerationResults> modifiedTiles;
	std::vector<NavigationMeshTileRegenerationResults> removedTiles;
};
  
using NavigationMeshDidFinishRegenerationCallback = std::function<void(NavigationMeshRegenerationChangeset)>;
	
struct ManagedFuncPackage {
  uint64_t didFinishRegeneratingFunc;
  uint64_t tileNavigationMeshGeometryUpdateFunc;
};
		
using NavigationMeshGeometryEntityHandle = uint64_t;

class NavigationMesh {
public:
  NavigationMesh();
  ~NavigationMesh();
  
  Result initialize(
	float maxWalkableSlope,
	float agentRadius,
	float agentHeight,
	float agentMaxClimb
	// TODO: Bounding box parameter. Only geometry entity polygons within these bounds will be rasterized.
  );
  
  // TODO: load navigation mesh from asset file.
  // void LoadNavigationMeshTiles();
  
  /**
   * @brief Creates a navigation geometry entity that will be rasterized on the navigation mesh after the next
   *        time that 'regenerateIfNeeded' is called.
   * @param transform the transform of the geometry entity in the navigation mesh's local space.
   * @return The handle to the geometry entity that was registered.
   */
  // TODO: Add flags & area id to be assigned to this geometry entity.
  Result registerNavigationMeshGeometryEntity(const float* transform, 
											  const unsigned short* triangles, 
											  uint32_t trianglesCount, 
											  const float* vertices,
											  uint32_t verticesCount,
											  NavigationMeshGeometryEntityHandle& handle);
  
  // Removes the navigation geometry entity. It will no longer be rasterized on the navigation mesh
  // after the next time 'regenerateIfNeeded' is called.
  Result unregisterNavigationMeshGeometryEntity(NavigationMeshGeometryEntityHandle handle);
  
  // Sets the transform of the navigation geometry entity corresponding to the handle in the navigation 
  // mesh's local space. The navigation mesh is unaffected until 'regenerateIfNeeded' is called.
  // Does nothing if the transform does not does not change.
  Result setNavigationMeshGeometryEntityTransform(NavigationMeshGeometryEntityHandle handle, const float* newTransform);
  
  // Sets the geometry of the navigation geometry entity corresponding to the provided handle.
  // Does NOT take effect until regenerateIfNeeded is called.
  Result setNavigationMeshGeometryEntityGeometry(NavigationMeshGeometryEntityHandle handle, 
											   const unsigned short* newTriangles, 
											   uint32_t trianglesCount, 
											   const float* newVertices, 
											   uint32_t verticesCount);
  
  /**
   * @brief Regenerate the navigation mesh tiles, if needed.
   */
  Result regenerateIfNeeded(NavigationMeshDidFinishRegenerationCallback&& didFinishRegenerationCallback);
  
  // If a path is found, pathPoints is set to the next numPathPoints points along that path. If foundPathPointsCount is 0, no path exists
  // oc
  /**
   * @brief Finds the path between two points on the navigation mesh. If foundPathPointsCount is 0, no path was found between the two points.
   * @param fromPoint .
   * @param toPoint .
   * @param maxPathPointsCount .
   * @param pathPoints .
   * @param foundPathPointsCount the number of points outputted in pathPoints. If 0, no path exists between the two points.
   * @return The result
   */
  Result findPath(const float* fromPoint, const float* toPoint, uint32_t maxPathPointsCount, float* pathPoints, uint32_t& foundPathPointsCount);

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
	// TODO: Maybe pointers to neighbors?
	// The field below is only updated when regenerateIfNeeded is called.
	std::unordered_map<NavigationMeshGeometryEntityHandle, std::vector<unsigned short>> intersectedGeometryEntityTris;
  };

  struct Geometry {
	  std::vector<unsigned short> triangles;
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
  };
  
  void cleanup();
  TileCoordinates tileCoordinatesForLocalPosition(float x, float y, float z);
  TileKey keyForTileCoordinates(TileCoordinates coordinates);
  TileKey keyForTileCoordinates(int32_t tx, int32_t ty);
  NavigationMeshTile* tileForTileCoordinates(int32_t tx, int32_t ty);
  NavigationMeshTile* createTileAtCoordinates(int32_t tx, int32_t ty);
  void clearTileNavigationMesh(NavigationMeshTile* tile);
  TileNavMeshGenStatus buildTileNavigationMesh(NavigationMeshTile* tile, unsigned char*& navMeshData, int& navMeshDataSize);
  void CalculateTileAABB(NavigationMeshTile* tile, float* AABBMin, float* AABBMax);

  std::unordered_map<NavigationMeshGeometryEntityHandle, NavigationMeshGeometryEntity> geometryEntities_;
  // 0 is reserved for the "Invalid" handle.
  std::atomic_uint64_t nextHandle_{1};
  
  std::unordered_set<NavigationMeshGeometryEntityHandle> queuedSpatiallyChangedGeometryEntities_;
  std::unordered_map<NavigationMeshGeometryEntityHandle, std::vector<TileKey>> queuedUnregisteredGeometryEntities_;
  
  rcConfig tileConfig_;
  rcHeightfield* tileHeightfield_;
  rcCompactHeightfield* tileCompactHeightfield_;
  rcContourSet* tileContourSet_;
  rcPolyMesh* tilePolyMesh_;
  rcPolyMeshDetail* tilePolyMeshDetail_;
  dtNavMesh* recastNavMesh_;
  dtNavMeshQuery* recastNavQuery_;
  rcContext recastContext_;
  
  float cellSize_;
  float cellHeight_;
  float maxWalkableSlope_;
  float agentRadius_;
  float agentHeight_;
  float agentMaxClimb_;
  float maxEdgeLen_;
  
  int32_t maxTiles_;
  int32_t maxPolysPerTile_;
  
  float navMeshBoundsY_[2];
  
  std::unordered_map<TileKey, NavigationMeshTile> tiles_;
  
  struct RegenerationCandidateTile {
	  NavigationMeshTile* tile;
	  bool isNewlyCreated;
  };
  // TODO: Sort these by time since first dirty, and rebuild longest first.
  std::unordered_map<TileKey, RegenerationCandidateTile> regenCandidateTiles_;
};

} // namespace pathfinding