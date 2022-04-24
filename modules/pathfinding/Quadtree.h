#pragma once

#include <algorithm>
#include <assert.h>
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>

using QuadtreeCellRef = uint32_t;

template<typename T>
class Quadtree {
public:
	Quadtree(float cellSize, int depth) {
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

	QuadtreeCellRef AddCellAt(int32_t x, int32_t y, T object) {
		auto iter = cellCoordinatesMap_.find(keyForCellCoordinates(x, y));
		if (iter != cellCoordinatesMap_.end()) {
			cells_[iter->second].object = object;
			return iter->second;
		}

		const int32_t halfSize = size_ >> 1;
		int32_t xRange[2] = { -halfSize, halfSize };
		int32_t yRange[2] = { -halfSize, halfSize };

		int currentDepth = 0;
		QuadtreeNodeRef currentNodeRef = 0;
		while (currentDepth < (depth_ - 1)) {
			const int32_t midX = (xRange[0] + xRange[1]) >> 1;
			const int32_t midY = (yRange[0] + yRange[1]) >> 1;
			if (nodes_[currentNodeRef].firstChild == -1) {
				nodes_[currentNodeRef].firstChild = AllocateChildrenNodes(currentNodeRef);
			}
			const bool isLeft = x < midX;
			const bool isBelow = y < midY;
			currentNodeRef = nodes_[currentNodeRef].firstChild + (isLeft ^ isBelow) + (isBelow << 1);
			xRange[isLeft] = midX;
			yRange[isBelow] = midY;
			currentDepth++;
		}

		QuadtreeCellRef cellRef;
		{
			const int32_t midX = (xRange[0] + xRange[1]) >> 1;
			const int32_t midY = (yRange[0] + yRange[1]) >> 1;
			if (nodes_[currentNodeRef].firstChild == -1) {
				nodes_[currentNodeRef].firstChild = AllocateChildrenCells(currentNodeRef, midX, midY);
			}
			const bool isLeft = x < midX;
			const bool isBelow = y < midY;
			cellRef = nodes_[currentNodeRef].firstChild + (isLeft ^ isBelow) + (isBelow << 1);
			QuadtreeCell& cell = cells_[cellRef];
			cell.isEmpty = false;
			cell.object = object;

			cellCoordinatesMap_[keyForCellCoordinates(x, y)] = cellRef;
		}

		return cellRef;
	}

	void RemoveCell(QuadtreeCellRef cellRef) {
		QuadtreeCell& cell = cells_.at(cellRef);
		QuadtreeNodeRef currentNodeRef = cell.parent;
		cell.isEmpty = true;
		cellCoordinatesMap_.erase(keyForCellCoordinates(cell.x, cell.y));
		{
			const int firstChildIndex = nodes_[currentNodeRef].firstChild;
			// Check if this node has any descendant cells left.
			for (int i = 0; i < 4; ++i) {
				if (!cells_[firstChildIndex + i].isEmpty) {
					return;
				}
			}

			DeallocateChildrenCells(currentNodeRef);
			nodes_[currentNodeRef].firstChild = -1;
			currentNodeRef = nodes_[currentNodeRef].parent;
		}

		while (currentNodeRef >= 0) {
			const int firstChildIndex = nodes_[currentNodeRef].firstChild;
			// Check if this node has any descendant cells left.
			for (int i = 0; i < 4; ++i) {
				if (nodes_[firstChildIndex + i].firstChild > -1) {
					return;
				}
			}

			DeallocateChildrenNodes(currentNodeRef);
			nodes_[currentNodeRef].firstChild = -1;
			currentNodeRef = nodes_[currentNodeRef].parent;
		}
	}

	bool ObjectForCellRef(QuadtreeCellRef cellRef, T*& object) {
		if (cellRef > 0 && cellRef < cells_.size() && !cells_[cellRef].isEmpty) {
			object = &cells_[cellRef].object;
			return true;
		}
		return false;
	}

	QuadtreeCellRef GetCellRefForCoordinates(int32_t x, int32_t y) {
		const CellCoordinatesKey key = keyForCellCoordinates(x, y);
		const std::unordered_map<CellCoordinatesKey, QuadtreeCellRef>::iterator iter = cellCoordinatesMap_.find(key);
		if (iter != cellCoordinatesMap_.end()) {
			return iter->second;
		}
		return 0;
	}

	void GetCellCoordinatesForPosition(const float* pos, int32_t& x, int32_t& y) {
		x = (int32_t)(floorf(pos[0] / cellSize_));
		y = (int32_t)(floorf(pos[2] / cellSize_));
	}

	bool GetCoordinatesForCellRef(QuadtreeCellRef cellRef, int32_t& x, int32_t& y) {
		if (cellRef > 0 && cellRef < cells_.size() && !cells_[cellRef].isEmpty) {
			x = cells_[cellRef].x;
			y = cells_[cellRef].y;
			return true;
		}
		return false;
	}

	void QueryNearestNeighbourCells(const float* point, std::function<float(QuadtreeCellRef, T&, float)> action) {
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
			QueryCandidateNode queryCandidate = dfsStack.back();
			//std::cout << minSqrDist << std::endl;
			//std::cout << queryCandidate.nodeRef << ", depth: " << queryCandidate.depth << ", sqrDist: " << queryCandidate.sqrDist 
			//	<< ", first child: " << nodes_[queryCandidate.nodeRef].firstChild << std::endl;
			dfsStack.pop_back();

			if (!(queryCandidate.sqrDist < minSqrDist)) {
				//std::cout << "not possible" << std::endl;
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

			if (queryCandidate.depth == (depth_ - 1)) {
				for (int i = 0; i < 4; ++i) {
					const QuadtreeCellRef cellRef = firstChildRef + i;
					minSqrDist = std::min(minSqrDist, action(cellRef, cells_[cellRef].object, minSqrDist));
				}
			}
			else {
				for (int i = 0; i < 4; ++i) {
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
					childQueryCandidate.sqrDist = diff[0] * diff[0] + diff[1] * diff[1];

					if (childQueryCandidate.sqrDist < minSqrDist) {
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

	QuadtreeNodeRef AllocateChildrenNodes(const QuadtreeNodeRef parentNodeRef) {
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

	void DeallocateChildrenNodes(const QuadtreeNodeRef parentNodeRef) {
		freeNodeSlots_.push(nodes_[parentNodeRef].firstChild);
	}

	QuadtreeCellRef AllocateChildrenCells(const QuadtreeNodeRef parentNodeRef, int32_t midX, int32_t midY) {
		const QuadtreeCell upperRightCell = { true, parentNodeRef, midX, midY, {} };
		const QuadtreeCell upperLeftCell = { true, parentNodeRef, midX - 1, midY, {} };
		const QuadtreeCell lowerLeftCell = { true, parentNodeRef, midX - 1, midY - 1, {} };
		const QuadtreeCell lowerRightCell = { true, parentNodeRef, midX, midY - 1, {} };
		QuadtreeCellRef freeCellSlot;
		if (freeCellSlots_.empty()) {
			freeCellSlot = cells_.size();
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

	void DeallocateChildrenCells(const QuadtreeNodeRef parentNodeRef) {
		freeCellSlots_.push(nodes_[parentNodeRef].firstChild);
	}

	static uint64_t keyForCellCoordinates(int32_t x, int32_t y) {
		uint64_t cellCoordsKey;
		char* tileCoordsKeyPtr = reinterpret_cast<char*>(&cellCoordsKey);
		memcpy(tileCoordsKeyPtr, &x, sizeof(x));
		memcpy(tileCoordsKeyPtr + sizeof(x), &y, sizeof(y));
		return cellCoordsKey;
	}
};