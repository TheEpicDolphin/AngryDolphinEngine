#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

using QuadtreeCellRef = uint32_t;

template<typename T>
class Quadtree {
public:
	Quadtree(float cellSize, int depth);

	QuadtreeCellRef AddCellAt(int x, int y, T object);

	void RemoveCell(QuadtreeCellRef cellRef);

	T& ObjectForCellRef(QuadtreeCellRef cellRef);

	QuadtreeCellRef GetCellRefAtLocation(int x, int y);

	void QueryAroundPoint(const float* point, std::function<, float>);

private:
	using QuadtreeNodeRef = int32_t;
	using CellCoordinatesKey = uint64_t;

	// Non-leaf nodes.
	struct QuadtreeNode {
		int parent;

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
		int parent;
		int32_t x;
		int32_t y;
		T object;
	};

	std::vector<QuadtreeNode> nodes_;
	std::queue<int> freeNodeSlots_;
	std::vector<QuadtreeCell> cells_;
	std::queue<int> freeCellSlots_;

	// Used for fast cell retrieval by coordinates.
	std::unordered_map<CellCoordinatesKey, QuadtreeCellRef> cellCoordinatesMap_;

	int depth_;
	int size_;
	float cellSize_;

	void AllocateChildrenNodes(const QuadtreeNodeRef parentNodeRef);
	void DeallocateChildrenNodes(const QuadtreeNodeRef parentNodeRef);
	void AllocateChildrenCells(const QuadtreeNodeRef parentNodeRef, int x, int y);
	void DeallocateChildrenCells(const QuadtreeNodeRef parentNodeRef);
};