#pragma once

#include <stdint.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "AABB.h"

namespace pathfinding {
	using GeometryPrimitiveRef = uint32_t;
	using NavigableSurfaceGroupRef = uint32_t;
	using NavigableSurfaceIndex = uint32_t;

	struct GeometryPrimitiveInfo {
		float* vertices;
		uint32_t verticesCount;
		int16_t* triangles;
		uint32_t trianglesCount;
	};

	struct NavigableSurfaceInfo {
		float toGroupLocalTransform[16];
		GeometryPrimitiveRef geometryPrimitive;
		char areaType;
	};

	struct NavigableSurfaceTriangle {
		int16_t indices[3];

	};

	class NavigableSurfaceRegistry {
	public:
		NavigableSurfaceRegistry();

		GeometryPrimitiveRef addGeometryPrimitive(GeometryPrimitiveInfo geometryPrimitive);

		NavigableSurfaceGroupRef createNavigableSurfaceGroup(const float* worldTransform);

		void setNavigableSurfaceGroupWorldTransform(
			NavigableSurfaceGroupRef groupRef,
			const float* newWorldTransform);

		void destroyNavigableSurfaceGroup(NavigableSurfaceGroupRef groupRef);

		void setNavigableSurfacesForGroup(
			NavigableSurfaceGroupRef groupRef,
			NavigableSurfaceInfo* surfaces,
			uint32_t surfacesCount);

		void setNavigableSurfacesForWorldGroup(
			NavigableSurfaceInfo* surfaces,
			uint32_t surfacesCount);

		void clear();

		std::vector<NavigableSurfaceGroupRef> pinAllGroupsAndGet();

		void unpinGroup(NavigableSurfaceGroupRef groupRef);

		void getGroupRelativeTransform(
			NavigableSurfaceGroupRef groupRef,
			float toWorldTransform[16]);

		void getAllIntersectingNavigableSurfaces(
			NavigableSurfaceGroupRef groupRef,
			const AABB worldAABB,
			std::vector<NavigableSurfaceIndex>& intersectingNavSurfaces);

		void foreachNavigableSurfaceTriangle(
			NavigableSurfaceGroupRef groupRef,
			NavigableSurfaceIndex navSurfaceIndex,
			const float* toLocalTransform,
			std::function<>
		)

		void queryNavigableSurfaceGeometry(
			NavigableSurfaceGroupRef groupRef,
			NavigableSurfaceIndex navSurfaceIndex,
			const float* toLocalTransform,
			std::vector<float>& transformedVertices,
			int16_t*& triangles,
			uint32_t& trianglesCount,
			char& areaType);

	private:
		struct GeometryPrimitive {
			uint32_t verticesIndex;
			uint16_t verticesCount;
			uint32_t trianglesIndex;
			uint16_t trianglesCount;
		};

		std::vector<GeometryPrimitive> geometryPrimitives_;
		std::vector<float> vertices_;
		std::vector<int16_t> triangles_;

		struct NavigableSurface {
			float toGroupLocalTransform[16];
			GeometryPrimitiveRef geometryPrimitiveRef;
			char areaType;

			// Below fields are recalculated whenever the group worldTransform
			// is modified.
			float toWorldTransform[16];
			AABB worldBounds;
		};

		void CalculateSurfaceToWorldTransform(const float* groupToWorldTransform, NavigableSurface& surface);
		void CalculateSurfaceGeometryWorldBounds(NavigableSurface& surface);

		struct NavigableSurfaceGroup {
			float toWorldTransform[16];
			std::vector<NavigableSurface> navigableSurfaces;
			int pinCount = 0;
			bool destroyOnceUnpinned = false;
		};

		// 0 is reserved for the world-space group handle.
		std::atomic_uint32_t nextGroupHandle_{ 1 };
		std::unordered_map<NavigableSurfaceGroupRef, NavigableSurfaceGroup> groups_;

		std::shared_mutex geometryPrimitivesMutex_;
		std::shared_mutex groupsMutex_;
		std::shared_mutex groupDataMutex_;
	};
}

