#include "Quadtree.h"

template<typename T>
Quadtree<T>::Quadtree(float cellSize, int depth) {
	cellSize_ = cellSize;
	depth_ = depth;
	size_ = 1 << (2 * depth_);

	nodes_.reserve(65);
	nodes_.push_back({ -1, -1 });
}

template<typename T>
QuadtreeCellRef Quadtree<T>::AddCellAt(int x, int y, T object) {
	int currentDepth = 0;
	int halfSize = size_ >> 1;
	int xRange[2] = { -halfSize, halfSize };
	int yRange[2] = { -halfSize, halfSize };
	Quadtree<T>::QuadtreeNodeRef currentNodeRef = 0;
	while (currentDepth < depth_) {
		int midX = (xRange[0] + xRange[1]) >> 1;
		int midY = (yRange[0] + yRange[1]) >> 1;
		if (nodes_[currentNodeRef]->firstNode == -1) {
			AllocateChildrenNodes(rootNodeRef);
		}

		const bool isLeft = x < midX;
		const bool isBelow = y < midY;
		currentNodeRef = nodes_[currentNodeRef]->firstNode + (isLeft ^ isBelow) + (isBelow << 1);
		xRange[isLeft] = midX;
		xRange[isBelow] = midY;
		currentDepth++;
	}

	QuadtreeCellRef cellRef = 0;
	if (freeCellSlots_.empty()) {
		cellRef = cells_.size();
		cells_.push_back({ currentNodeRef, x, y, object });
	}
	else {
		cellRef = freeCellSlots_.front();
		cells_[cellRef] = { currentNodeRef, x, y, object };
		freeCellSlots_.pop();
	}

	return cellRef;
}

template<typename T>
void Quadtree<T>::RemoveCell(QuadtreeCellRef cellRef) {
	const QuadtreeCell& cell = cells_.at(cellRef);
	int currentDepth = depth_;
	QuadtreeNodeRef currentNodeRef = cell.parent;
	while (currentDepth > 0) {
		int midX = (xRange[0] + xRange[1]) >> 1;
		int midY = (yRange[0] + yRange[1]) >> 1;
		if (nodes_[currentNodeRef]->firstNode == -1) {
			AllocateChildrenNodes(rootNodeRef);
		}

		const bool isLeft = x < midX;
		const bool isBelow = y < midY;
		currentNodeRef = nodes_[currentNodeRef]->firstNode + (isLeft ^ isBelow) + (isBelow << 1);
		xRange[isLeft] = midX;
		xRange[isBelow] = midY;
		currentDepth++;
	}
}

template<typename T>
T& Quadtree<T>::ObjectForCellRef(QuadtreeCellRef cellRef) {
	return cells_.at(cellRef).object;
}

template<typename T>
void Quadtree<T>::AllocateChildrenNodes(const Quadtree<T>::QuadtreeNodeRef parentNodeRef) {
	if (freeNodeSlots_.empty()) {
		nodes_.push_back({ parentNodeRef, -1 });
		nodes_.push_back({ parentNodeRef, -1 });
		nodes_.push_back({ parentNodeRef, -1 });
		nodes_.push_back({ parentNodeRef, -1 });
	}
	else {
		const int freeNodeSlot = freeNodeSlots_.front();
		nodes_[freeNodeSlot] = { parentNodeRef, -1 };
		nodes_[freeNodeSlot + 1] = { parentNodeRef, -1 };
		nodes_[freeNodeSlot + 2] = { parentNodeRef, -1 };
		nodes_[freeNodeSlot + 3] = { parentNodeRef, -1 };
		freeNodeSlots_.pop();
	}
}

template<typename T>
void Quadtree<T>::DeallocateChildrenNodes(const Quadtree<T>::QuadtreeNodeRef parentNodeRef) {

}