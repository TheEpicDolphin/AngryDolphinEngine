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
			AllocateChildrenNodes(currentNodeRef);
		}

		const bool isLeft = x < midX;
		const bool isBelow = y < midY;
		currentNodeRef = nodes_[currentNodeRef]->firstNode + (isLeft ^ isBelow) + (isBelow << 1);
		xRange[isLeft] = midX;
		xRange[isBelow] = midY;
		currentDepth++;
	}

	AllocateChildrenCells(currentNodeRef, midX, midY);
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
	QuadtreeNodeRef currentNodeRef = cell.parent;
	cell.isEmpty = true;
	// Check if this node has any descendant cells left.
	{
		const int firstChildIndex = nodes_[currentNodeRef].firstChild;
		for (int i = 0; i < 4; i++) {
			if (!cells_[firstChildIndex + i].isEmpty) {
				return;
			}
		}
	}
	DeallocateChildrenCells(currentNodeRef);
	nodes_[currentNodeRef].firstChild = -1;
	currentNodeRef = nodes_[currentNodeRef].parent;
	while (currentNodeRef >= 0) {
		const int firstChildIndex = nodes_[currentNodeRef].firstChild;

		// Check if this node has any descendant cells left.
		for (int i = 0; i < 4; i++) {
			if (nodes_[firstChildIndex + i].firstChild > -1) {
				return;
			}
		}

		DeallocateChildrenNodes(currentNodeRef);
		nodes_[currentNodeRef].firstChild = -1;
		currentNodeRef = nodes_[currentNodeRef].parent;
	}
}

template<typename T>
T& Quadtree<T>::ObjectForCellRef(QuadtreeCellRef cellRef) {
	return cells_.at(cellRef).object;
}

template<typename T>
QuadtreeCellRef Quadtree<T>::GetCellRefAtLocation(int x, int y) {

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

template<typename T>
void Quadtree<T>::AllocateChildrenCells(const Quadtree<T>::QuadtreeNodeRef parentNodeRef, int x, int y) {
	if (freeCellSlots_.empty()) {
		nodes_.push_back({ true, parentNodeRef, x, y, {} });
		nodes_.push_back({ true, parentNodeRef, x, y, {} });
		nodes_.push_back({ true, parentNodeRef, x, y, {} });
		nodes_.push_back({ true, parentNodeRef, x, y, {} });
	}
	else {
		const int freeCellSlot = freeCellSlots_.front();
		nodes_[freeCellSlot] = { true, parentNodeRef, x, y, {} };
		nodes_[freeCellSlot + 1] = { true, parentNodeRef, x, y, {} };
		nodes_[freeCellSlot + 2] = { true, parentNodeRef, x, y, {} };
		nodes_[freeCellSlot + 3] = { true, parentNodeRef, x, y, {} };
		freeCellSlots_.pop();
	}
}

template<typename T>
void Quadtree<T>::DeallocateChildrenCells(const QuadtreeNodeRef parentNodeRef) {

}