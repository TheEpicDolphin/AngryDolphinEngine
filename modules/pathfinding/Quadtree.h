#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>

using QuadtreeCellRef = uint32_t;

template<typename T>
class Quadtree {
public:
	Quadtree(float cellSize, int depth);

	QuadtreeCellRef AddCellAt(int32_t x, int32_t y, T object);

	void RemoveCell(QuadtreeCellRef cellRef);

	bool ObjectForCellRef(QuadtreeCellRef cellRef, T*& object);

	QuadtreeCellRef GetCellRefForCoordinates(int32_t x, int32_t y);

	void GetCellCoordinatesForPosition(const float* pos, int32_t& x, int32_t& y);

	bool GetCoordinatesForCellRef(QuadtreeCellRef cellRef, int32_t& x, int32_t& y);

	void QueryNearestNeighbourCells(const float* point, std::function<float(T&, float)> action);

private:
	using QuadtreeNodeRef = int32_t;
	using CellCoordinatesKey = uint64_t;

	// Non-leaf nodes.
	struct QuadtreeNode {
		QuadtreeNodeRef parent;

		// Children go in counter-clockwise order. If -1, this node has no children.
		//					|
		//					|
		//	firstChild + 1	|	firstChild
		//	----------------|------------------
		//	firstChild + 2	|	firstChild + 3
		//					|	
		//					|
		int firstChild;
	};

	// Leaf nodes.
	struct QuadtreeCell {
		bool isEmpty;
		QuadtreeNodeRef parent;
		int32_t x;
		int32_t y;
		T object;
	};

	std::vector<QuadtreeNode> nodes_;
	std::queue<QuadtreeNodeRef> freeNodeSlots_;
	std::vector<QuadtreeCell> cells_;
	std::queue<QuadtreeCellRef> freeCellSlots_;

	// Used for fast cell retrieval by coordinates.
	std::unordered_map<CellCoordinatesKey, QuadtreeCellRef> cellCoordinatesMap_;

	int depth_;
	int32_t size_;
	float cellSize_;

	QuadtreeNodeRef AllocateChildrenNodes(const QuadtreeNodeRef parentNodeRef);
	void DeallocateChildrenNodes(const QuadtreeNodeRef parentNodeRef);
	QuadtreeCellRef AllocateChildrenCells(const QuadtreeNodeRef parentNodeRef, int32_t midX, int32_t midY);
	void DeallocateChildrenCells(const QuadtreeNodeRef parentNodeRef);
	CellCoordinatesKey keyForCellCoordinates(int32_t x, int32_t y);
};