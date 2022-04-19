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
bool Quadtree<T>::ObjectForCellRef(QuadtreeCellRef cellRef, T*& object) {
	if (cellRef > 0 && cellRef < cells_.size() && !cells_[cellRef].isEmpty) {
		object = &cells_[cellRef].object;
		return true;
	}
	return false;
}

template<typename T>
QuadtreeCellRef Quadtree<T>::GetCellRefForCoordinates(int32_t x, int32_t y) {
	const CellCoordinatesKey key = keyForCellCoordinates(x, y);
	const std::unordered_map<CellCoordinatesKey, QuadtreeCellRef>::iterator iter = cellCoordinatesMap_.find(key);
	if (iter != cellCoordinatesMap_.end()) {
		return iter->second;
	}
	return 0;
}

template<typename T>
void Quadtree<T>::GetCellCoordinatesForPosition(const float* pos, int32_t& x, int32_t& y) {
	x = (int32_t)(floorf(pos[0] / cellSize_));
	y = (int32_t)(floorf(pos[2] / cellSize_));
}

template<typename T>
bool Quadtree<T>::GetCoordinatesForCellRef(QuadtreeCellRef cellRef, int32_t& x, int32_t& y) {
	if (cellRef > 0 && cellRef < cells_.size() && !cells_[cellRef].isEmpty) {
		x = cells_[cellRef].x;
		y = cells_[cellRef].y;
		return true;
	}
	return false;
}

template<typename T>
void Quadtree<T>::QueryNearestNeighbourCells(const float* point, std::function<float(T&)> action) {
	/*
	int32_t x;
	int32_t y;
	GetCellCoordinatesForPosition(point, x, y);

	QuadtreeNodeRef containingNode;
	const QuadtreeCellRef cellRef = GetCellRefForCoordinates(x, y);
	if (cellRef) {
		containingNode = cells_[cellRef].parent;
	}
	*/

	float minSqrDist = FLT_MAX;

	struct QueryCandidateNode {
		int nodeRef;
		int depth;
		int32_t xBounds[2];
		int32_t yBounds[2];
		float sqrDist;

		bool operator > (const QueryCandidateNode& other) const
		{
			return (sqrDist > other.sqrDist);
		}
	};

	const int32_t halfSize = size_ >> 1;
	std::vector<QueryCandidateNode> dfsStack;
	// Start with root node.
	dfsStack.push_back({ 0, 0, {}, {}, 0.0f });
	dfsStack[0].xBounds[0] = -halfSize;
	dfsStack[0].xBounds[1] = halfSize;
	dfsStack[0].yBounds[0] = -halfSize;
	dfsStack[0].yBounds[1] = halfSize;

	std::vector<QueryCandidateNode> childQueryCandidates;
	childQueryCandidates.reserve(4);

	while (!dfsStack.empty()) {
		const QueryCandidateNode queryCandidate = dfsStack.back();
		dfsStack.pop_back();

		if (!(queryCandidate.sqrDist < minSqrDist)) {
			// It is no longer possible for this node to have contents
			// closer than minSqrDist to point.
			continue;
		}

		int firstChildRef = nodes_[queryCandidate.nodeRef].firstChild;
		if (firstChildRef < 0) {
			// This node has no children. Skip.
			continue;
		}

		const int32_t midX = (queryCandidate.xBounds[0] + queryCandidate.xBounds[1]) >> 1;
		const int32_t midY = (queryCandidate.yBounds[0] + queryCandidate.yBounds[1]) >> 1;

		if (queryCandidate.depth == depth_ - 1) {
			for (int i = 0; i < 4; i++) {
				minSqrDist = std::min(minSqrDist, action(cells_[firstChildRef + i].object));
			}
		} else {
			for (int i = 0; i < 4; i++) {
				QueryCandidateNode childQueryCandidate = {
					firstChildRef + i,
					queryCandidate.depth + 1,
					{},
					{},
					0.0f,
				};

				const bool isRightOfCenter = i % 3 == 0;
				childQueryCandidate.xBounds[!isRightOfCenter] = midX;
				childQueryCandidate.xBounds[isRightOfCenter] = queryCandidate.xBounds[isRightOfCenter];

				const bool isAboveCenter = i < 2;
				childQueryCandidate.yBounds[!isAboveCenter] = midY;
				childQueryCandidate.yBounds[isAboveCenter] = queryCandidate.yBounds[isAboveCenter];

				float pointClamped[2] = {
					std::min(std::max(childQueryCandidate.xBounds[0] * cellSize_, point[0]), childQueryCandidate.xBounds[1] * cellSize_),
					std::min(std::max(childQueryCandidate.yBounds[0] * cellSize_, point[2]), childQueryCandidate.yBounds[1] * cellSize_),
				};
				const float diff[2] = { point[0] - pointClamped[0], point[2] - pointClamped[1] };
				const float sqrDist = diff[0] * diff[0] + diff[1] * diff[1];

				if (queryCandidate.sqrDist < minSqrDist) {
					// Only consider this child node if it is less than minSqrDist to point.
					childQueryCandidates.push_back(childQueryCandidate);
				}
			}

			if (!childQueryCandidates.empty()) {
				// Sort nodes by descending order of squared distance to point. This is the order we want to
				// append them to the queue to ensure a depth-first search traversal.
				std::sort(childQueryCandidates.begin(), childQueryCandidates.end(), std::greater<QueryCandidateNode>());
				dfsStack.insert(dfsStack.end(), childQueryCandidates.begin(), childQueryCandidates.end());
				childQueryCandidates.clear();
			}
		}
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