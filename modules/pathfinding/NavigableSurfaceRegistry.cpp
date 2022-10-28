#include "NavigableSurfaceRegistry.h"

namespace pathfinding {
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
	 * @brief Transforms the vector4 using the given transformation matrix.
	 * @param transformMatrix4x4 The input transformation matrix.
	 * @param vector4 The input vector4 to transform.
	 * @param transformedVector4 The output transformed vector4.
	 */
	inline void TransformVector4(const float* transformMatrix4x4, const float* vector4, float* transformedVector4) {
		transformedVector4[0] = transformMatrix4x4[0] * vector4[0] + transformMatrix4x4[4] * vector4[1] +
			transformMatrix4x4[8] * vector4[2] + transformMatrix4x4[12];
		transformedVector4[1] = transformMatrix4x4[1] * vector4[0] + transformMatrix4x4[5] * vector4[1] +
			transformMatrix4x4[9] * vector4[2] + transformMatrix4x4[13];
		transformedVector4[2] = transformMatrix4x4[2] * vector4[0] + transformMatrix4x4[6] * vector4[1] +
			transformMatrix4x4[10] * vector4[2] + transformMatrix4x4[14];
		transformedVector4[3] = transformMatrix4x4[3] * vector4[0] + transformMatrix4x4[7] * vector4[1] +
			transformMatrix4x4[11] * vector4[2] + transformMatrix4x4[15];
	}

	/**
	 * @brief Transforms the matrix using the given transformation matrix.
	 * @param transformMatrix4x4 The input transformation matrix.
	 * @param matrix4x4 The input matrix to transform.
	 * @param transformedMatrix4x4 The output transformed matrix.
	 */
	inline void TransformMatrix4x4(const float* transformMatrix4x4, const float* matrix4x4, float* transformedMatrix4x4) {
		for (int i = 0; i < 4; ++i) {
			TransformVector4(transformMatrix4x4, matrix4x4 + 4 * i, transformedMatrix4x4 + 4 * i);
		}
	}

	NavigableSurfaceRegistry::NavigableSurfaceRegistry() {
		NavigableSurfaceGroup worldGroup;
		// Set world transform to identity matrix.
		std::memset(worldGroup.toWorldTransform, 0, 16 * sizeof(float));
		for (int i = 0; i < 4; ++i) {
			worldGroup.toWorldTransform[5 * i] = 1;
		}
		groups_.insert({ 0, worldGroup });
	}

	GeometryPrimitiveRef NavigableSurfaceRegistry::addGeometryPrimitive(GeometryPrimitiveInfo geometryPrimitive) {
		GeometryPrimitive primitive;
		primitive.verticesIndex = vertices_.size();
		primitive.verticesCount = geometryPrimitive.verticesCount;
		primitive.trianglesIndex = triangles_.size();
		primitive.trianglesCount = geometryPrimitive.trianglesCount;

		const std::unique_lock<std::shared_mutex> writeLock(geometryPrimitivesMutex_);
		vertices_.insert(
			vertices_.end(),
			geometryPrimitive.vertices,
			geometryPrimitive.vertices + 3 * geometryPrimitive.verticesCount
		);
		triangles_.insert(
			triangles_.end(),
			geometryPrimitive.triangles,
			geometryPrimitive.triangles + 3 * geometryPrimitive.trianglesCount
		);
		geometryPrimitives_.push_back(primitive);
		return geometryPrimitives_.size() - 1;
	}

	NavigableSurfaceGroupRef NavigableSurfaceRegistry::createNavigableSurfaceGroup(const float* worldTransform) {
		NavigableSurfaceGroup newGroup;
		std::memcpy(newGroup.toWorldTransform, worldTransform, 16 * sizeof(float));
		const NavigableSurfaceGroupRef groupRef =  nextGroupHandle_.fetch_add(1, std::memory_order_relaxed);

		const std::unique_lock<std::shared_mutex> writeLock(groupsMutex_);
		groups_.insert({ groupRef, newGroup });
		return groupRef;
	}

	void NavigableSurfaceRegistry::setNavigableSurfaceGroupWorldTransform(NavigableSurfaceGroupRef groupRef, const float* newWorldTransform) {
		const std::shared_lock<std::shared_mutex> groupsReadLock(groupsMutex_);
		auto foundGroupIter = groups_.find(groupRef);
		if (foundGroupIter == groups_.end()) {
			return;
		}

		NavigableSurfaceGroup& group = foundGroupIter->second;
		const std::unique_lock<std::shared_mutex> groupDataWriteLock(groupDataMutex_);
		std::memcpy(group.toWorldTransform, newWorldTransform, 16 * sizeof(float));

		// Iterate surfaces and modify AABB and world transforms.
		for (auto surfaceIter = group.navigableSurfaces.begin();
			surfaceIter != group.navigableSurfaces.end();
			++surfaceIter) {
			CalculateSurfaceToWorldTransform(group.toWorldTransform, *surfaceIter);
			CalculateSurfaceGeometryWorldBounds(*surfaceIter);
		}
	}

	void NavigableSurfaceRegistry::destroyNavigableSurfaceGroup(NavigableSurfaceGroupRef groupRef) {
		const std::unique_lock<std::shared_mutex> writeLock(groupsMutex_);
		auto foundGroupIter = groups_.find(groupRef);
		if (foundGroupIter == groups_.end()) {
			return;
		}

		NavigableSurfaceGroup& group = foundGroupIter->second;
		const std::unique_lock<std::shared_mutex> groupDataWriteLock(groupDataMutex_);
		if (group.pinCount > 0) {
			group.destroyOnceUnpinned = true;
		} else {
			groups_.erase(groupRef);
		}
	}

	void NavigableSurfaceRegistry::setNavigableSurfacesForGroup(
		NavigableSurfaceGroupRef groupRef,
		NavigableSurfaceInfo* surfaces,
		uint32_t surfacesCount) {
		const std::shared_lock<std::shared_mutex> groupsReadLock(groupsMutex_);
		auto foundGroupIter = groups_.find(groupRef);
		if (foundGroupIter == groups_.end()) {
			return;
		}

		NavigableSurfaceGroup& group = foundGroupIter->second;
		const std::unique_lock<std::shared_mutex> groupDataWriteLock(groupDataMutex_);
		group.navigableSurfaces.clear();
		group.navigableSurfaces.resize(surfacesCount);
		for (uint32_t i = 0; i < surfacesCount; ++i) {
			NavigableSurface& surface = group.navigableSurfaces[i];
			std::memcpy(surface.toGroupLocalTransform, surfaces[i].toGroupLocalTransform, 16 * sizeof(float));
			surface.geometryPrimitiveRef = surfaces[i].geometryPrimitive;
			surface.areaType = surfaces[i].areaType;
			CalculateSurfaceToWorldTransform(group.toWorldTransform, surface);
			CalculateSurfaceGeometryWorldBounds(surface);
		}
	}

	void NavigableSurfaceRegistry::setNavigableSurfacesForWorldGroup(
		NavigableSurfaceInfo* surfaces,
		uint32_t surfacesCount) {
		setNavigableSurfacesForGroup(0, surfaces, surfacesCount);
	}

	void NavigableSurfaceRegistry::clear() {
		{
			const std::unique_lock<std::shared_mutex> writeLock(groupsMutex_);
			groups_.clear();
		}

		{
			const std::unique_lock<std::shared_mutex> writeLock(geometryPrimitivesMutex_);
			geometryPrimitives_.clear();
			vertices_.clear();
			triangles_.clear();
		}
	}

	std::vector<NavigableSurfaceGroupRef> NavigableSurfaceRegistry::pinAllGroupsAndGet() {
		const std::shared_lock<std::shared_mutex> groupsReadLock(groupsMutex_);
		std::vector<NavigableSurfaceGroupRef> groups;
		for (auto groupsIter = groups_.begin(); groupsIter != groups_.end(); groupsIter++) {
			groupsIter->second.pinCount += 1;
			groups.push_back(groupsIter->first);
		}
	}

	void NavigableSurfaceRegistry::unpinGroup(NavigableSurfaceGroupRef groupRef) {
		const std::shared_lock<std::shared_mutex> groupsReadLock(groupsMutex_);
		auto foundGroupIter = groups_.find(groupRef);
		if (foundGroupIter == groups_.end()) {
			return;
		}

		NavigableSurfaceGroup& group = foundGroupIter->second;
		const std::unique_lock<std::shared_mutex> groupDataWriteLock(groupDataMutex_);
		group.pinCount -= 1;
		if (group.destroyOnceUnpinned && group.pinCount == 0) {
			groups_.erase(groupRef);
		}
	}

	void NavigableSurfaceRegistry::getGroupToWorldTransform(
		NavigableSurfaceGroupRef groupRef,
		const float toWorldTransform[16],
		float toWorldTransform[16]) {
		const std::shared_lock<std::shared_mutex> groupsReadLock(groupsMutex_);
		auto foundGroupIter = groups_.find(groupRef);
		if (foundGroupIter == groups_.end()) {
			return;
		}

		const std::shared_lock<std::shared_mutex> groupDataReadLock(groupDataMutex_);
		std::memcpy(toWorldTransform, foundGroupIter->second.toWorldTransform, 16 * sizeof(float));
	}

	void NavigableSurfaceRegistry::getAllIntersectingNavigableSurfaces(
		NavigableSurfaceGroupRef groupRef,
		const AABB worldAABB,
		std::vector<NavigableSurfaceIndex>& intersectingNavSurfaces) {
		const std::shared_lock<std::shared_mutex> groupsReadLock(groupsMutex_);
		auto foundGroupIter = groups_.find(groupRef);
		if (foundGroupIter == groups_.end()) {
			return;
		}

		// Iterate surfaces and check for intersections with worldAABB.
		NavigableSurfaceGroup& group = foundGroupIter->second;
		const std::shared_lock<std::shared_mutex> groupDataReadLock(groupDataMutex_);
		for (std::size_t i = 0; i < group.navigableSurfaces.size(); ++i) {
			const NavigableSurface& navSurface = group.navigableSurfaces[i];
			if (navSurface.worldBounds.Intersects(worldAABB)) {
				intersectingNavSurfaces.push_back(i);
			}
		}
	}

	void NavigableSurfaceRegistry::queryNavigableSurfaceGeometry(
		NavigableSurfaceGroupRef groupRef,
		NavigableSurfaceIndex navSurfaceIndex,
		const float* toLocalTransform,
		std::vector<float>& transformedVertices,
		int16_t*& triangles,
		uint32_t& trianglesCount,
		char& areaType) {
		const std::shared_lock<std::shared_mutex> groupsReadLock(groupsMutex_);
		auto foundGroupIter = groups_.find(groupRef);
		if (foundGroupIter == groups_.end()) {
			return;
		}

		const NavigableSurface& navSurface = foundGroupIter->second.navigableSurfaces[navSurfaceIndex];
		transformedVertices.clear();
		{
			const std::shared_lock<std::shared_mutex> readLock(geometryPrimitivesMutex_);
			const GeometryPrimitive& geometryPrimitive = geometryPrimitives_[navSurface.geometryPrimitiveRef];
			const uint32_t verticesCount = geometryPrimitive.verticesCount;
			for (uint32_t vi = 0; vi < 3 * verticesCount; vi += 3) {
				float worldSpaceVertex[3];
				TransformPoint(navSurface.toWorldTransform, &vertices_[vi], worldSpaceVertex);
				float transformedVertex[3];
				TransformPoint(toLocalTransform, worldSpaceVertex, transformedVertex);
				transformedVertices.push_back(transformedVertex[0]);
				transformedVertices.push_back(transformedVertex[1]);
				transformedVertices.push_back(transformedVertex[2]);
			}

			triangles = &triangles_[geometryPrimitive.trianglesIndex];
			trianglesCount = geometryPrimitive.trianglesCount;
			areaType = navSurface.areaType;
		}
	}

	void NavigableSurfaceRegistry::CalculateSurfaceToWorldTransform(
		const float* groupToWorldTransform,
		NavigableSurface& surface) {
		TransformMatrix4x4(groupToWorldTransform, surface.toGroupLocalTransform, surface.toWorldTransform);
	}

	void NavigableSurfaceRegistry::CalculateSurfaceGeometryWorldBounds(
		NavigableSurface& surface) {
		const std::shared_lock<std::shared_mutex> readLock(geometryPrimitivesMutex_);
		GeometryPrimitive& geometryPrimitive = geometryPrimitives_[surface.geometryPrimitiveRef];
		uint32_t vi = geometryPrimitive.verticesIndex;
		float* vertex = &vertices_[vi];
		TransformPoint(surface.toWorldTransform, vertex, surface.worldBounds.min);
		std::memcpy(surface.worldBounds.max, surface.worldBounds.min, 3 * sizeof(float));
		vi += 3;
		
		while (vi < 3 * geometryPrimitive.verticesCount) {
			vertex = &vertices_[vi];
			float transformedVertex[3];
			TransformPoint(surface.toWorldTransform, vertex, transformedVertex);
			surface.worldBounds.min[0] = std::min(surface.worldBounds.min[0], transformedVertex[0]);
			surface.worldBounds.min[1] = std::min(surface.worldBounds.min[1], transformedVertex[1]);
			surface.worldBounds.min[2] = std::min(surface.worldBounds.min[2], transformedVertex[2]);

			surface.worldBounds.max[0] = std::max(surface.worldBounds.max[0], transformedVertex[0]);
			surface.worldBounds.max[1] = std::max(surface.worldBounds.max[1], transformedVertex[1]);
			surface.worldBounds.max[2] = std::max(surface.worldBounds.max[2], transformedVertex[2]);
			vi += 3;
		}
	}
}
