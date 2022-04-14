#include "Quadtree.h"

#include <assert.h> 

uint64_t keyForCellCoordinates(int32_t x, int32_t y) {
	uint64_t cellCoordsKey;
	char* tileCoordsKeyPtr = reinterpret_cast<char*>(&cellCoordsKey);
	memcpy(tileCoordsKeyPtr, &x, sizeof(x));
	memcpy(tileCoordsKeyPtr + sizeof(x), &y, sizeof(y));
	return cellCoordsKey;
}

template<typename T>
Quadtree<T>::Quadtree(float cellSize, int depth) {
	assert(depth > 0);
	cellSize_ = cellSize;
	depth_ = depth;
	size_ = 1 << depth_;

	// Push the root node as the first element.
	nodes_.reserve(65);
	nodes_.push_back({ -1, -1 });

	// Push a dummy cell as the first element, because
	// QuadtreeCellRef == 0 is reserved for invalid
	// cell refs.
	cells_.reserve(65);
	cells_.push_back({});
}

template<typename T>
QuadtreeCellRef Quadtree<T>::AddCellAt(int32_t x, int32_t y, T object) {
	const int32_t halfSize = size_ >> 1;
	int32_t xRange[2] = { -halfSize, halfSize };
	int32_t yRange[2] = { -halfSize, halfSize };

	int currentDepth = 0;
	QuadtreeNodeRef currentNodeRef = 0;
	while (currentDepth < (depth_ - 1)) {
		const int32_t midX = (xRange[0] + xRange[1]) >> 1;
		const int32_t midY = (yRange[0] + yRange[1]) >> 1;
		if (nodes_[currentNodeRef].firstChild == -1) {
			AllocateChildrenNodes(currentNodeRef);
		}
		const bool isLeft = x < midX;
		const bool isBelow = y < midY;
		currentNodeRef = nodes_[currentNodeRef].firstChild + (isLeft ^ isBelow) + (isBelow << 1);
		xRange[isLeft] = midX;
		xRange[isBelow] = midY;
		currentDepth++;
	}

	QuadtreeCellRef cellRef = nodes_[currentNodeRef].firstChild;
	{
		const int32_t midX = (xRange[0] + xRange[1]) >> 1;
		const int32_t midY = (yRange[0] + yRange[1]) >> 1;
		if (cellRef == -1) {
			cellRef = AllocateChildrenCells(currentNodeRef, midX, midY);
		}
		const bool isLeft = x < midX;
		const bool isBelow = y < midY;
		cellRef += (isLeft ^ isBelow) + (isBelow << 1);
		QuadtreeCell& cell = cells_[cellRef];
		cell.isEmpty = false;
		cell.object = object;

		cellCoordinatesMap_[keyForCellCoordinates(x, y)] = cellRef;
	}

	return cellRef;
}

template<typename T>
void Quadtree<T>::RemoveCell(QuadtreeCellRef cellRef) {
	const QuadtreeCell& cell = cells_.at(cellRef);
	QuadtreeNodeRef currentNodeRef = cell.parent;
	cell.isEmpty = true;
	cellCoordinatesMap_.erase(keyForCellCoordinates(cell.x, cell.y));
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
QuadtreeCellRef Quadtree<T>::GetCellRefForCoordinates(int32_t x, int32_t y) {
	const CellCoordinatesKey key = keyForCellCoordinates(x, y);
	const std::unordered_map<CellCoordinatesKey, QuadtreeCellRef>::iterator iter = cellCoordinatesMap_.find(key);
	if (iter != cellCoordinatesMap_.end()) {
		return iter->second;
	} else {
		return 0;
	}
}

template<typename T>
Quadtree<T>::QuadtreeNodeRef Quadtree<T>::AllocateChildrenNodes(const QuadtreeNodeRef parentNodeRef) {
	QuadtreeNodeRef freeNodeSlot;
	if (freeNodeSlots_.empty()) {
		freeNodeSlot = nodes_.size();
		nodes_.push_back({ parentNodeRef, -1 });
		nodes_.push_back({ parentNodeRef, -1 });
		nodes_.push_back({ parentNodeRef, -1 });
		nodes_.push_back({ parentNodeRef, -1 });
	}
	else {
		freeNodeSlot = freeNodeSlots_.front();
		nodes_[freeNodeSlot] = { parentNodeRef, -1 };
		nodes_[freeNodeSlot + 1] = { parentNodeRef, -1 };
		nodes_[freeNodeSlot + 2] = { parentNodeRef, -1 };
		nodes_[freeNodeSlot + 3] = { parentNodeRef, -1 };
		freeNodeSlots_.pop();
	}
	return freeNodeSlot;
}

template<typename T>
void Quadtree<T>::DeallocateChildrenNodes(const QuadtreeNodeRef parentNodeRef) {
	freeNodeSlots_.push(nodes_[parentNodeRef].firstChild);
}

template<typename T>
QuadtreeCellRef Quadtree<T>::AllocateChildrenCells(const QuadtreeNodeRef parentNodeRef, int32_t midX, int32_t midY) {
	const QuadtreeCell upperRightCell = { true, parentNodeRef, midX, midY, {} };
	const QuadtreeCell upperLeftCell = { true, parentNodeRef, midX - 1, midY, {} };
	const QuadtreeCell lowerLeftCell = { true, parentNodeRef, midX - 1, midY - 1, {} };
	const QuadtreeCell lowerRightCell = { true, parentNodeRef, midX, midY - 1, {} };
	QuadtreeCellRef freeCellSlot;
	if (freeCellSlots_.empty()) {
		freeCellSlot = nodes_.size();
		cells_.push_back(upperRightCell);
		cells_.push_back(upperLeftCell);
		cells_.push_back(lowerLeftCell);
		cells_.push_back(lowerRightCell);
	}
	else {
		freeCellSlot = freeCellSlots_.front();
		cells_[freeCellSlot] = upperRightCell;
		cells_[freeCellSlot + 1] = upperLeftCell;
		cells_[freeCellSlot + 2] = lowerLeftCell;
		cells_[freeCellSlot + 3] = lowerRightCell;
		freeCellSlots_.pop();
	}
	return freeCellSlot;
}

template<typename T>
void Quadtree<T>::DeallocateChildrenCells(const QuadtreeNodeRef parentNodeRef) {
	freeCellSlots_.push(nodes_[parentNodeRef].firstChild);
}