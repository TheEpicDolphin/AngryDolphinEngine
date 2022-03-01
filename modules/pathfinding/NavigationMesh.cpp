/**
 * (c) Facebook, Inc. and its affiliates. Confidential and proprietary.
 */

#include "NavigationMesh.h"

#include <Detour/Include/DetourNavMeshBuilder.h>

#include <iostream>

namespace pathfinding {

// Assumes transformMatrix4x4 has column-major order
void TransformPoint(const float* transformMatrix4x4, const float* point, float* transformedPoint) {
  transformedPoint[0] = transformMatrix4x4[0] * point[0] + transformMatrix4x4[4] * point[1] + transformMatrix4x4[8] * point[2] + transformMatrix4x4[12];
  transformedPoint[1] = transformMatrix4x4[1] * point[0] + transformMatrix4x4[5] * point[1] + transformMatrix4x4[9] * point[2] + transformMatrix4x4[13];
  transformedPoint[2] = transformMatrix4x4[2] * point[0] + transformMatrix4x4[6] * point[1] + transformMatrix4x4[10] * point[2] + transformMatrix4x4[14];
}

bool IsApproximatelyEqual(const float* values1, uint32_t count1, const float* values2, uint32_t count2, float epsilon = 0.0001f) {
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

bool IsEqual(const unsigned short* values1, uint32_t count1, const unsigned short* values2, uint32_t count2) {
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

std::vector<float> tileNavigationMeshGeometry(rcPolyMesh polyMesh){
	const int nvp = polyMesh.nvp;
	const float cs = polyMesh.cs;
	const float ch = polyMesh.ch;
	const float* orig = polyMesh.bmin;
	
	std::vector<float> polyMeshVertices(3 * polyMesh.npolys);
	// TODO: area ids
	// std::vector<unsigned char> polyMeshAreas(polyMesh.npolys);
	
	for (int i = 0; i < polyMesh.npolys; ++i)
	{
		const unsigned short* p = &polyMesh.polys[i*nvp*2];
		//const unsigned char area = polyMesh.areas[i];		
		unsigned short vi[3];
		for (int j = 2; j < nvp; ++j)
		{
			if (p[j] == RC_MESH_NULL_IDX) break;
			vi[0] = p[0];
			vi[1] = p[j-1];
			vi[2] = p[j];
			for (int k = 0; k < 3; ++k)
			{
				const unsigned short* v = &polyMesh.verts[vi[k]*3];
				const float x = orig[0] + v[0]*cs;
				const float y = orig[1] + (v[1]+1)*ch;
				const float z = orig[2] + v[2]*cs;
				
				polyMeshVertices[3 * i] = x;
				polyMeshVertices[3 * i + 1] = y;
				polyMeshVertices[3 * i + 2] = z;
			}
		}
		//polyMeshAreas[i] = area;
	}

	return polyMeshVertices;
}

NavigationMesh::NavigationMesh() {
	tileHeightfield_ = 0;
	tileCompactHeightfield_ = 0;
	tileContourSet_ = 0;
	tilePolyMesh_ = 0;
	tilePolyMeshDetail_ = 0;
	recastNavMesh_ = 0;
	recastNavQuery_ = 0;
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
		std::cout << tilePolyMesh_ << std::endl;
		rcFreePolyMesh(tilePolyMesh_);
		tilePolyMesh_ = 0;
	}

	if (tilePolyMeshDetail_) {
		rcFreePolyMeshDetail(tilePolyMeshDetail_);
		tilePolyMeshDetail_ = 0;
	}
}

Result NavigationMesh::initialize(
	float maxWalkableSlope,
	float agentRadius,
	float agentHeight,
	float agentMaxClimb) {
  maxWalkableSlope_ = maxWalkableSlope;
  agentRadius_ = agentRadius;
  agentHeight_ = agentHeight;
  agentMaxClimb_ = agentMaxClimb;
  
  // According to the author of Recast, a good value for cell size is
  // 1/2 or 1/3 of the agent radius.
  cellSize_ = agentRadius / 3.0f;
  cellHeight_ = agentHeight / 2.5f;
  
  // The author of recast recommends agent radius * 8 for the max edge length.
  maxEdgeLen_ = agentRadius * 8.0f;
  
  // There are only 22 bits available to be divided among identifying tiles
  // and identifying polys within those tiles.
  // TODO: Take into account any user defined bounding box for the navigation mesh
  // to calculate this.
  const int32_t tileBits = 14;
  const int32_t polyBits = 22 - tileBits;
  maxTiles_ = 1 << tileBits;
  maxPolysPerTile_ = 1 << polyBits;
  
  navMeshBoundsY_[0] = -1000.0f;
  navMeshBoundsY_[1] = 1000.0f;
  
  dtFreeNavMesh(recastNavMesh_);
  recastNavMesh_ = dtAllocNavMesh();
  if (!recastNavMesh_)
  {
	recastContext_.log(RC_LOG_ERROR, "buildTiledNavigation: Failed to allocate navmesh.");
	return Result::kErrorOutOfMemory;
  }
  
  dtNavMeshParams navMeshParams;
  navMeshParams.orig[0] = 0;
  navMeshParams.orig[1] = 0;
  navMeshParams.orig[2] = 0;
  navMeshParams.tileWidth = kTileSize * cellSize_;
  navMeshParams.tileHeight = kTileSize * cellSize_;
  navMeshParams.maxTiles = maxTiles_;
  navMeshParams.maxPolys = maxPolysPerTile_;
  
  dtStatus status;
  status = recastNavMesh_->init(&navMeshParams);
  if (dtStatusFailed(status))
  {
	recastContext_.log(RC_LOG_ERROR, "buildTiledNavigation: Failed to init navmesh.");
	return Result::kErrorUnexpected;
  }

  recastNavQuery_ = dtAllocNavMeshQuery();
  status = recastNavQuery_->init(recastNavMesh_, 2048);
  if (dtStatusFailed(status))
  {
	recastContext_.log(RC_LOG_ERROR, "buildTiledNavigation: Failed to init Detour navmesh query");
	return Result::kErrorUnexpected;
  }
  
  // Init build configuration used for all tiles. These fields do not change after initialization.
  memset(&tileConfig_, 0, sizeof(tileConfig_));
  tileConfig_.cs = cellSize_;	// Voxel along x and z axis.
  tileConfig_.ch = cellHeight_; // Voxel height (y axis).
  tileConfig_.walkableSlopeAngle = maxWalkableSlope_;
  tileConfig_.walkableHeight = (int)ceilf(agentHeight_ / tileConfig_.ch);   // Voxel coordinates.
  tileConfig_.walkableClimb = (int)floorf(agentMaxClimb_ / tileConfig_.ch); // Voxel coordinates.
  tileConfig_.walkableRadius = (int)ceilf(agentRadius_ / tileConfig_.cs);   // Voxel coordinates.
  tileConfig_.maxEdgeLen = (int)(maxEdgeLen_ / tileConfig_.cs); 			// Voxel coordinates.
  tileConfig_.maxSimplificationError = kMaxSimplificationError;
  tileConfig_.minRegionArea = (int)rcSqr(kMinRegionSize);		// Note: area = size*size
  tileConfig_.mergeRegionArea = (int)rcSqr(kMergeRegionSize);	// Note: area = size*size
  tileConfig_.maxVertsPerPoly = (int)kMaxVertsPerPoly;
  tileConfig_.tileSize = (int)kTileSize;
  tileConfig_.borderSize = tileConfig_.walkableRadius + 3;
  tileConfig_.width = tileConfig_.tileSize + tileConfig_.borderSize * 2;
  tileConfig_.height = tileConfig_.tileSize + tileConfig_.borderSize * 2;
  tileConfig_.detailSampleDist = kDetailSampleDistance < 0.9f ? 0 : tileConfig_.cs * kDetailSampleDistance;
  tileConfig_.detailSampleMaxError = tileConfig_.ch * kDetailSampleMaxError;

  std::cout << "tile size: " << tileConfig_.tileSize << std::endl;
  std::cout << "cell size: " << tileConfig_.cs << std::endl;

  return Result::kOk;
}

Result NavigationMesh::registerNavigationMeshGeometryEntity(const float* transform, 
															const unsigned short* triangles, 
															uint32_t trianglesCount, 
															const float* vertices,
															uint32_t verticesCount,
															NavigationMeshGeometryEntityHandle& handle) {
  const NavigationMeshGeometryEntityHandle geometryEntityHandle = nextHandle_.fetch_add(1, std::memory_order_relaxed);
  
  geometryEntities_.insert({geometryEntityHandle, {0}});
  
  setNavigationMeshGeometryEntityTransform(geometryEntityHandle, transform);
  setNavigationMeshGeometryEntityGeometry(geometryEntityHandle, triangles, trianglesCount, vertices, verticesCount);
  
  // Set handle to the registered geometry entity's handle.
  handle = geometryEntityHandle;
  
  return Result::kOk;
}

Result NavigationMesh::unregisterNavigationMeshGeometryEntity(NavigationMeshGeometryEntityHandle handle) {
  if (geometryEntities_.find(handle) != geometryEntities_.end()) {
	// Remove pending changes to this geometry entity, if there are any.
	queuedSpatiallyChangedGeometryEntities_.erase(handle);
	
	NavigationMeshGeometryEntity& geometryEntity = geometryEntities_[handle];
	queuedUnregisteredGeometryEntities_.insert({ handle, geometryEntity.intersectedTiles });
	geometryEntities_.erase(handle);
	
	return Result::kOk;
  } else {
	return Result::kErrorKeyNotFound;
  }
}

Result NavigationMesh::setNavigationMeshGeometryEntityTransform(NavigationMeshGeometryEntityHandle handle, 
																const float* newTransform) {
  std::unordered_map<NavigationMeshGeometryEntityHandle, NavigationMeshGeometryEntity>::iterator iter = geometryEntities_.find(handle);
  if (iter != geometryEntities_.end()) {
	NavigationMeshGeometryEntity& geometryEntity = iter->second;
	if (!IsApproximatelyEqual(geometryEntity.transform, 16, newTransform, 16)) {
	  memcpy(geometryEntity.transform, newTransform, 16 * sizeof(float));
	  queuedSpatiallyChangedGeometryEntities_.insert(handle);
	}
	return Result::kOk;
  } else {
	return Result::kErrorKeyNotFound;
  }
}

Result NavigationMesh::setNavigationMeshGeometryEntityGeometry(NavigationMeshGeometryEntityHandle handle,
															   const unsigned short* newTriangles, 
															   uint32_t newTrianglesCount, 
															   const float* newVertices,
															   uint32_t newVerticesCount) {
  std::unordered_map<NavigationMeshGeometryEntityHandle, NavigationMeshGeometryEntity>::iterator iter = geometryEntities_.find(handle);
  if (iter != geometryEntities_.end()) {
	NavigationMeshGeometryEntity& geometryEntity = iter->second;
	if (!IsEqual(geometryEntity.geometry.triangles.data(), geometryEntity.geometry.triangles.size(), newTriangles, newTrianglesCount) 
		|| !IsApproximatelyEqual(geometryEntity.geometry.vertices.data(), geometryEntity.geometry.vertices.size(), newVertices, newVerticesCount)) {
	  geometryEntity.geometry.triangles.clear();
	  geometryEntity.geometry.triangles.insert(geometryEntity.geometry.triangles.begin(), newTriangles, newTriangles + 3 * newTrianglesCount);
	  geometryEntity.geometry.vertices.clear();
	  geometryEntity.geometry.vertices.insert(geometryEntity.geometry.vertices.begin(), newVertices, newVertices + 3 * newVerticesCount);
	  queuedSpatiallyChangedGeometryEntities_.insert(handle);
	}
	return Result::kOk;
  } else {
	return Result::kErrorKeyNotFound;
  }
}

Result NavigationMesh::regenerateIfNeeded(NavigationMeshDidFinishRegenerationCallback&& didFinishRegenerationCallback){  
  // Phase 1: Process removed geometry entities
  for (auto iter = queuedUnregisteredGeometryEntities_.begin(); iter != queuedUnregisteredGeometryEntities_.end(); ++iter) {
	const NavigationMeshGeometryEntityHandle handle = iter->first;
	std::vector<NavigationMeshTile*>& previouslyIntersectedTiles = iter->second;
	
	// For each tile previously intersected by this geometry entity, remove this geometry entity's
	// handle from its dictionary of intersections, and consider the tile as "affected".
	for (NavigationMeshTile* tile : previouslyIntersectedTiles) {
	  tile->intersectedGeometryEntityTris.erase(handle);
	  const uint64_t tileKey = keyForTileCoordinates(tile->coordinates);
	  if (dirtyTiles_.find(tileKey) == dirtyTiles_.end()) {
		dirtyTiles_.insert({tileKey, tile});
	  }
	}
  }
  
  // Phase 2: Process geometry entity geometry changes.
  for (auto iter = queuedSpatiallyChangedGeometryEntities_.begin(); iter != queuedSpatiallyChangedGeometryEntities_.end(); ++iter) {
	const NavigationMeshGeometryEntityHandle handle = *iter;
	NavigationMeshGeometryEntity& geometryEntity = geometryEntities_[handle];
	
	// Find tiles that were previously intersected by this geometry entity. They are likely
	// affected by the spatial changes applied to this geometry entity.
	for (NavigationMeshTile* tile : geometryEntity.intersectedTiles) {
		
	  // Assume that this geometry entity no longer intersects this tile.
	  // If we are wrong, it will be corrected in the next step anyway, where 
	  // we check for new geometry entity-tile intersections after the vertices 
	  // are transformed.
	  tile->intersectedGeometryEntityTris.erase(handle);
	  
	  const uint64_t tileKey = keyForTileCoordinates(tile->coordinates);
	  if (dirtyTiles_.find(tileKey) == dirtyTiles_.end()) {
		dirtyTiles_.insert({tileKey, tile});
	  }
	}
	
	// Clear previously intersected tiles. We will calculate new ones.
	geometryEntity.intersectedTiles.clear();
	
	// Transform geometry verts to navigation mesh local space using new transform and/or new geometry.
	geometryEntity.transformedGeometryVertices.clear();
	for (std::size_t i = 0; i < geometryEntity.geometry.vertices.size(); i+=3) {
	  float vertex[3] = { geometryEntity.geometry.vertices[i], geometryEntity.geometry.vertices[i + 1], geometryEntity.geometry.vertices[i + 2] };
	  //std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
	  float transformedVertex[3];
	  TransformPoint(geometryEntity.transform, vertex, transformedVertex);
	  geometryEntity.transformedGeometryVertices.push_back(transformedVertex[0]); // push_back x
	  geometryEntity.transformedGeometryVertices.push_back(transformedVertex[1]); // push_back y
	  geometryEntity.transformedGeometryVertices.push_back(transformedVertex[2]); // push_back z
	  //std::cout << "(" << transformedVertex[0] << ", " << transformedVertex[1] << ", " << transformedVertex[2] << ")" << std::endl;
	}
	
	// Find tiles affected by the geometry entity after its geometry is transformed.
	for (std::size_t i = 0; i < geometryEntity.geometry.triangles.size(); i+=3) {
	  const std::size_t vi0 = geometryEntity.geometry.triangles[i];
	  const std::size_t vi1 = geometryEntity.geometry.triangles[i + 1];
	  const std::size_t vi2 = geometryEntity.geometry.triangles[i + 2];
	  
	  const float v0x = geometryEntity.transformedGeometryVertices[3 * vi0];
	  const float v0z = geometryEntity.transformedGeometryVertices[3 * vi0 + 2];
	  
	  const float v1x = geometryEntity.transformedGeometryVertices[3 * vi1];
	  const float v1z = geometryEntity.transformedGeometryVertices[3 * vi1 + 2];
	  
	  const float v2x = geometryEntity.transformedGeometryVertices[3 * vi2];
	  const float v2z = geometryEntity.transformedGeometryVertices[3 * vi2 + 2];
	  
	  // We calculate an AABB for the triangle, expanded by the tile border size.
	  //
	  // It is faster to expand the triangle AABB by the border size and check for intersections
	  // with surrounding tiles than it is to expand each tile AABB by the border size and check if
	  // it intersects this triangle.
	  const float bs = tileConfig_.borderSize * tileConfig_.cs;
	  const float triAABBMin[2] = { fmin(fmin(v0x, v1x), v2x) - bs, fmin(fmin(v0z, v1z), v2z) - bs };
	  const float triAABBMax[2] = { fmax(fmax(v0x, v1x), v2x) + bs, fmax(fmax(v0z, v1z), v2z) + bs };
	 
	  const TileCoordinates minTileCoords = tileCoordinatesForLocalPosition(triAABBMin[0], 0, triAABBMin[1]);
	  const TileCoordinates maxTileCoords = tileCoordinatesForLocalPosition(triAABBMax[0], 0, triAABBMax[1]);
	  
	  // This may produce some false positives for tiles-triangle intersections, but that is ok because
	  // the false positives are later filtered out internally by Recast during the rasterization process.
	  for (int32_t tx = minTileCoords.tx; tx <= maxTileCoords.tx; tx++) {
		for (int32_t ty = minTileCoords.ty; ty <= maxTileCoords.ty; ty++) {
			NavigationMeshTile* tile = tileForTileCoordinates(tx, ty);
			
			auto geometryEntityTrisIter = tile->intersectedGeometryEntityTris.find(handle);
			if (geometryEntityTrisIter != tile->intersectedGeometryEntityTris.end()) {
				geometryEntityTrisIter->second.push_back(vi0);
				geometryEntityTrisIter->second.push_back(vi1);
				geometryEntityTrisIter->second.push_back(vi2);
			} else {
				tile->intersectedGeometryEntityTris[handle] = { (unsigned short)vi0, (unsigned short)vi1, (unsigned short)vi2 };
			}
			
			geometryEntity.intersectedTiles.push_back(tile);
			
			const uint64_t tileKey = keyForTileCoordinates(tile->coordinates);
			if (dirtyTiles_.find(tileKey) == dirtyTiles_.end()) {
		      dirtyTiles_.insert({tileKey, tile});
			}
		}
	  }
	}
  }

  // TODO: Phase 3: Process geometry entity flag/area changes.
  
  std::cout << dirtyTiles_.size() << std::endl;
  for (std::unordered_map<uint64_t, NavigationMeshTile*>::iterator dirtyTileIter = dirtyTiles_.begin(); dirtyTileIter != dirtyTiles_.end(); ++dirtyTileIter) {
	  NavigationMeshTile* dirtyTile = dirtyTileIter->second;
	  std::cout << "(" << dirtyTile->coordinates.tx << ", " << dirtyTile->coordinates.ty << ")" << std::endl;
  }

  // Phase 4: Regenerate dirty tiles.
  std::vector<uint64_t> cleanedTiles;
  std::vector<TileRegenerationResults> navMeshRegenerationResults;
  for (std::unordered_map<uint64_t, NavigationMeshTile*>::iterator dirtyTileIter = dirtyTiles_.begin(); dirtyTileIter != dirtyTiles_.end(); ++dirtyTileIter) {
	const uint64_t dirtyTileKey = dirtyTileIter->first;
	NavigationMeshTile* dirtyTile = dirtyTileIter->second;
	
	std::cout << "(" << dirtyTile->coordinates.tx << ", " << dirtyTile->coordinates.ty << ")" << std::endl;

	// First, destroy the affected tile. If necessary, we will rebuild it.
	destroyTile(dirtyTile);
	
	// Don't even bother rebuilding the tile if there is no geometry intersecting it.
	if (!dirtyTile->intersectedGeometryEntityTris.empty()) {
	  int navmeshDataSize = 0;
	  std::cout << "rebuilding tile..." << std::endl;
	  unsigned char* navmeshData = rebuildTile(dirtyTile, navmeshDataSize);
	  std::cout << "rebuilt tile SUCCEEDED" << std::endl;
	  if (navmeshData) {
		dtStatus status = recastNavMesh_->addTile(navmeshData, navmeshDataSize, DT_TILE_FREE_DATA, 0, 0);
		if (dtStatusFailed(status)) {
		  // Free the navmesh data because we failed to add the tile.
		  dtFree(navmeshData);
		} else {
		  // Tile was rebuilt successfully
			std::cout << "REBUILT SUCCESSFULLY" << std::endl;
		  std::vector<float> polymeshVertices = tileNavigationMeshGeometry(*tilePolyMesh_);
		  navMeshRegenerationResults.push_back({dirtyTile->coordinates.tx, dirtyTile->coordinates.ty, polymeshVertices});
		  cleanedTiles.push_back(dirtyTileKey);
		}
	  }
	} else {
	  cleanedTiles.push_back(dirtyTileKey);
	  // Delete this tile. We dont have to worry about removing this tile from the intersectedTiles vector of 
	  // intersected geometry entities because it wasn't intersecting any triangles of any geometry entities.
	  tiles_.erase(dirtyTileKey);
	}
  }

  // Delete cleaned tiles from dirtyTiles_.
  for (uint64_t cleanedTileKey : cleanedTiles) {
	  dirtyTiles_.erase(cleanedTileKey);
  }
  
  // Clear queued events.
  queuedSpatiallyChangedGeometryEntities_.clear();
  queuedUnregisteredGeometryEntities_.clear();
  
  // Report that the navigation mesh finished regenerating
  didFinishRegenerationCallback(navMeshRegenerationResults);
	
  return Result::kOk;
}

// Returns the existing tile containing localPos. If there is no such tile exists, one is created.
NavigationMesh::NavigationMeshTile* NavigationMesh::tileForLocalPosition(float x, float y, float z) {
  const TileCoordinates coordinates = tileCoordinatesForLocalPosition(x, y, z);
  return tileForTileCoordinates(coordinates.tx, coordinates.ty);
}

NavigationMesh::TileCoordinates NavigationMesh::tileCoordinatesForLocalPosition(float x, float y, float z) {
  const float ts = tileConfig_.tileSize * tileConfig_.cs;
  const int32_t tx = (int32_t)(floorf(x / ts));
  const int32_t ty = (int32_t)(floorf(z / ts));
  TileCoordinates coordinates = { tx, ty };
  return coordinates;
}

uint64_t NavigationMesh::keyForTileCoordinates(TileCoordinates coordinates){
  uint64_t tileKey;
  char* tileKeyPtr = reinterpret_cast<char*>(&tileKey);
  memcpy(tileKeyPtr, &coordinates.tx, sizeof(coordinates.tx));
  memcpy(tileKeyPtr + sizeof(coordinates.tx), &coordinates.ty, sizeof(coordinates.ty));
  return tileKey;
}

NavigationMesh::NavigationMeshTile* NavigationMesh::tileForTileCoordinates(int32_t tx, int32_t ty) {
  TileCoordinates coordinates = { tx, ty };
  const uint64_t tileKey = keyForTileCoordinates(coordinates);
  std::unordered_map<uint64_t, NavigationMeshTile>::iterator tileIter = tiles_.find(tileKey);
  if (tileIter == tiles_.end()) {
	return createTileAtCoordinates(coordinates);
  }
  return &tileIter->second;
}

NavigationMesh::NavigationMeshTile* NavigationMesh::createTileAtCoordinates(TileCoordinates coordinates) {
  const uint64_t tileKey = keyForTileCoordinates(coordinates);
  std::pair<std::unordered_map<uint64_t, NavigationMeshTile>::iterator, bool> insertion = tiles_.insert({tileKey, {coordinates, {}}});
  return &(insertion.first->second);
}

void NavigationMesh::destroyTile(NavigationMeshTile* tile) {
  recastNavMesh_->removeTile(recastNavMesh_->getTileRefAt(tile->coordinates.tx, tile->coordinates.ty, 0), 0, 0);
}

unsigned char* NavigationMesh::rebuildTile(NavigationMeshTile* tile, int& navDataSize) {
  // Find axis aligned bounding box of the tile. 
  float tileAABBMin[3];
  float tileAABBMax[3];
  CalculateTileAABB(tile, tileAABBMin, tileAABBMax);
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
  std::cout << "cleaning up..." << std::endl;
  cleanup();
  std::cout << "cleaned up!" << std::endl;

  // Reset build times gathering.
  recastContext_.resetTimers();
	
  // Start the build process.
  recastContext_.startTimer(RC_TIMER_TOTAL);
	
  recastContext_.log(RC_LOG_PROGRESS, "Building tile at: (%d, %d)", tile->coordinates.tx, tile->coordinates.ty);
	
  // Allocate voxel heightfield where we rasterize our input data to.
  tileHeightfield_ = rcAllocHeightfield();
  if (!tileHeightfield_)
  {
  	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Out of memory for heightfield allocation for 'tileHeightfield_'.");
  	return 0;
  }
  if (!rcCreateHeightfield(&recastContext_, *tileHeightfield_, tileConfig_.width, tileConfig_.height, tileConfig_.bmin, tileConfig_.bmax, tileConfig_.cs, tileConfig_.ch))
  {
  	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to create solid heightfield.");
  	return 0;
  }
  
  // Rasterize all intersecting geometry entities onto the tile.
  for (auto& intersectedGeometryEntityTris : tile->intersectedGeometryEntityTris) {
	const NavigationMeshGeometryEntityHandle handle = intersectedGeometryEntityTris.first;
	const NavigationMeshGeometryEntity& geometryEntity = geometryEntities_[handle];
	const std::vector<unsigned short>& intersectedTriangles = intersectedGeometryEntityTris.second;
	
	// Not all of these vertices will be used. Only those indexed by intersectedTriangles will be used by Recast.
	const float* verts = geometryEntity.transformedGeometryVertices.data();
	const std::size_t nverts = geometryEntity.transformedGeometryVertices.size() / 3;
	const unsigned short* ctris = intersectedTriangles.data();
	const std::size_t nctris = intersectedTriangles.size() / 3;

	//memset(m_triareas, 0, nctris*sizeof(unsigned char));
    //rcMarkWalkableTriangles(&recastContext_, tileConfig_.walkableSlopeAngle, verts, nverts, ctris, nctris, m_triareas);
	for (std::size_t i = 0; i < geometryEntity.transformedGeometryVertices.size(); i+=3) {
		float x = geometryEntity.transformedGeometryVertices[i];
		float y = geometryEntity.transformedGeometryVertices[i + 1];
		float z = geometryEntity.transformedGeometryVertices[i + 2];
		//std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
	}
	
	unsigned char* triareas = new unsigned char[nctris];
	memset(triareas, RC_WALKABLE_AREA, nctris * sizeof(unsigned char));

	// TODO: Set area ids of triangles for this entity
	bool success = rcRasterizeTriangles(
		&recastContext_, 
		verts,   					// Vertices
		nverts,  					// Number of vertices
		ctris,   					// Triangle indices
		triareas, 					// Area id corresponding to these triangles. TODO: Set this field to the area id of this geometry entity.
		nctris,  					// Number of triangles
		*tileHeightfield_, 			// Heightfield
		tileConfig_.walkableClimb	// Walkable climb
	);

	delete[] triareas;

	if (!success) {
	  recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to rasterize triangles.");
	  return 0;
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
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Out of memory for compact heightfield allocation for 'tileCompactHeightfield_'.");
	return 0;
  }
  if (!rcBuildCompactHeightfield(&recastContext_, tileConfig_.walkableHeight, tileConfig_.walkableClimb, *tileHeightfield_, *tileCompactHeightfield_)) {
	  recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to build compact heightfield.");
	return 0;
  }

  // Erode the walkable area by agent radius.
  /*
  if (!rcErodeWalkableArea(&recastContext_, tileConfig_.walkableRadius, *tileCompactHeightfield_)) {
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to erode.");
	return 0;
  }
  */

  // (Optional) Mark areas.
  //const ConvexVolume* vols = m_geom->getConvexVolumes();
  //for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i) {
  //	rcMarkConvexPolyArea(&recastContext_, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *tileCompactHeightfield_);
  //}
  
  // Prepare for Watershed region partitioning by calculating distance field along the walkable surface.
  if (!rcBuildDistanceField(&recastContext_, *tileCompactHeightfield_)) {
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
	return 0;
  }

  // Partition the walkable surface into simple regions without holes.
  if (!rcBuildRegions(&recastContext_, *tileCompactHeightfield_, tileConfig_.borderSize, tileConfig_.minRegionArea, tileConfig_.mergeRegionArea)) {
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
	return 0;
  }
  
  // Create contours.
  tileContourSet_ = rcAllocContourSet();
  if (!tileContourSet_) {
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Out of memory for contour set allocation for 'tileContourSet_'.");
	return 0;
  }
  if (!rcBuildContours(&recastContext_, *tileCompactHeightfield_, tileConfig_.maxSimplificationError, tileConfig_.maxEdgeLen, *tileContourSet_)) {
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to create contours.");
	return 0;
  }

  if (tileContourSet_->nconts == 0) {
	  std::cout << "NO CONTOURS" << std::endl;
	// Zero contours were produce. Return early because no navmesh will be generated for this tile.
	return 0;
  }
  std::cout << "CONTOUR MADE" << std::endl;
  // Build polygon navmesh from the contours.
  tilePolyMesh_ = rcAllocPolyMesh();
  if (!tilePolyMesh_) {
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Out of memory for poly mesh allocation for 'tilePolyMesh_'.");
	return 0;
  }
  
  if (!rcBuildPolyMesh(&recastContext_, *tileContourSet_, tileConfig_.maxVertsPerPoly, *tilePolyMesh_)) {
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to triangulate contours.");
	return 0;
  }

  // Build detail mesh.
  tilePolyMeshDetail_ = rcAllocPolyMeshDetail();
  if (!tilePolyMeshDetail_) {
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Out of memory for poly mesh detail allocation for 'tilePolyMeshDetail_'.");
	return 0;
  }
	
  if (!rcBuildPolyMeshDetail(&recastContext_, *tilePolyMesh_, *tileCompactHeightfield_,
  						   tileConfig_.detailSampleDist, tileConfig_.detailSampleMaxError,
							   *tilePolyMeshDetail_)) {
	recastContext_.log(RC_LOG_ERROR, "buildNavigation: Failed to build polymesh detail.");
	return 0;
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
  
  unsigned char* navData = 0;
  if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
	recastContext_.log(RC_LOG_ERROR, "Failed to build Detour navmesh.");
	return 0;
  }
  
  recastContext_.stopTimer(RC_TIMER_TOTAL);
  float tileBuildTime = recastContext_.getAccumulatedTime(RC_TIMER_TOTAL)/1000.0f;
  float tileMemoryUsage = navDataSize/1024.0f;
  
  recastContext_.log(RC_LOG_PROGRESS, ">> Tile Build Time: %f sec", tileBuildTime);
  recastContext_.log(RC_LOG_PROGRESS, ">> Tile Memory Usage: %f KB", tileMemoryUsage);
  recastContext_.log(RC_LOG_PROGRESS, ">> Tile polymesh has: %d vertices and %d polygons", tilePolyMesh_->nverts, tilePolyMesh_->npolys);
  
  return navData;
}

void NavigationMesh::CalculateTileAABB(NavigationMeshTile* tile, float* AABBMin, float* AABBMax) {
  const float ts = tileConfig_.tileSize * tileConfig_.cs;
  const float minX = tile->coordinates.tx * ts;
  const float minY = tile->coordinates.ty * ts;
  
  AABBMin[0] = minX;
  AABBMin[1] = navMeshBoundsY_[0];
  AABBMin[2] = minY;
  
  AABBMax[0] = minX + ts;
  AABBMax[1] = navMeshBoundsY_[1];
  AABBMax[2] = minY + ts;
}

Result NavigationMesh::findPath(const float* fromPoint, const float* toPoint, uint32_t maxPathPointsCount, float* pathPoints, uint32_t& foundPathPointsCount) {
  // TODO: Call recast methods to find path.
  return Result::kOk;
}

} // namespace logging
