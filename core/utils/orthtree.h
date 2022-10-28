// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <algorithm>
#include <assert.h>
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>
#include <float.h>
#include <cstring>
#include <cmath>

using OrthtreeCellRef = int32_t;

static const int ORTHTREE_CHILD_CELL_OFFSETS[8][3] = {
    { 0, 0, 0 },
    { -1, 0, 0 },
    { -1, 0, -1 },
    { 0, 0, -1 },
    { 0, -1, 0 },
    { -1, -1, 0 },
    { -1, -1, -1 },
    { 0, -1, -1 },
};

template <std::size_t Dims, typename T>
class orthtree {
private:
    static const int ORTHTREE_CHILD_COUNT = 1 << Dims;

public:
    orthtree() {
        static_assert(Dims > 0, "Orthtree must have at least 1 dimension.");
        static_assert(Dims <= 3, "Orthtree must have at most 3 dimensions.");
        depth_ = -1;
    }

    orthtree(float cellSize, uint32_t depth) {
        static_assert(Dims > 0, "Orthtree must have at least 1 dimension.");
        static_assert(Dims <= 3, "Orthtree must have at most 3 dimensions.");
        reset(cellSize, depth);
    }

    void reset(float cellSize, uint32_t depth) {
        assert(cellSize > 0);
        assert(depth > 0);
        cellSize_ = cellSize;
        depth_ = depth;
        size_ = 1 << depth_;

        nodes_.clear();
        freeNodeSlots_.clear();
        cells_.clear();
        freeCellSlots_.clear();
        cellCoordinatesMap_.clear();

        // Push the root node as the first element.
        nodes_.push_back({ -1, -1 });
    }

    OrthtreeCellRef AddCellAt(const int16_t coords[Dims], T object) {
        assert(depth > 0, "cell size and depth of the orthtree must be set before use.");
        CellCoordinatesKey coordsKey = keyForCellCoordinates(coords);
        auto iter = cellCoordinatesMap_.find(coordsKey);
        if (iter != cellCoordinatesMap_.end()) {
            cells_[iter->second].object = object;
            return iter->second;
        }

        const int16_t halfSize = size_ >> 1;
        int16_t ranges[Dims][2];
        for (int dim = 0; dim < Dims; ++dim) {
            ranges[dim][0] = -halfSize;
            ranges[dim][1] = halfSize;
        }
        int16_t mid[Dims];

        uint16_t currentDepth = 0;
        // Set current node ref to root.
        int32_t currentNodeRef = 0;
        while (currentDepth < depth_) {
            int32_t childOffset = 0;
            for (int dim = 0; dim < Dims; ++dim) {
                mid[dim] = (ranges[dim][0] + ranges[dim][1]) >> 1;
                const bool lessThanMid = coords[dim] < mid[dim];
                childOffset += (lessThanMid << dim);
                ranges[dim][lessThanMid] = mid[dim];
            }

            if (nodes_[currentNodeRef].firstChild == -1) {
                // This node has no children. Allocate them.
                const bool isCellDepth = currentDepth == (depth_ - 1);
                if (isCellDepth) {
                    nodes_[currentNodeRef].firstChild = AllocateChildrenCells(currentNodeRef, mid);
                }
                else {
                    nodes_[currentNodeRef].firstChild = AllocateChildrenNodes(currentNodeRef);
                }
            }

            currentNodeRef = nodes_[currentNodeRef].firstChild + childOffset;
            currentDepth++;
        }

        OrthtreeCell& cell = cells_[currentNodeRef];
        cell.isEmpty = false;
        cell.object = object;

        cellCoordinatesMap_[coordsKey] = currentNodeRef;
        return currentNodeRef;
    }

    void RemoveCell(OrthtreeCellRef cellRef) {
        assert(depth > 0, "cell size and depth of the orthtree must be set before use.");
        OrthtreeCell& cell = cells_.at(cellRef);
        OrthtreeInternalNodeRef currentNodeRef = cell.parent;
        cell.isEmpty = true;
        cellCoordinatesMap_.erase(keyForCellCoordinates(cell.coords));
        {
            const int firstChildIndex = nodes_[currentNodeRef].firstChild;
            // Check if this node has any descendant cells left.
            for (int i = 0; i < ORTHTREE_CHILD_COUNT; ++i) {
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
            for (int i = 0; i < ORTHTREE_CHILD_COUNT; ++i) {
                if (nodes_[firstChildIndex + i].firstChild > -1) {
                    return;
                }
            }

            DeallocateChildrenNodes(currentNodeRef);
            nodes_[currentNodeRef].firstChild = -1;
            currentNodeRef = nodes_[currentNodeRef].parent;
        }
    }

    bool ObjectForCellRef(OrthtreeCellRef cellRef, T*& object) {
        assert(depth > 0, "cell size and depth of the orthtree must be set before use.");
        if (cellRef > 0 && cellRef < cells_.size() && !cells_[cellRef].isEmpty) {
            object = &cells_[cellRef].object;
            return true;
        }
        return false;
    }

    OrthtreeCellRef GetCellRefForCoordinates(int16_t* coords) {
        assert(depth > 0, "cell size and depth of the orthtree must be set before use.");
        const CellCoordinatesKey key = keyForCellCoordinates(coords);
        const std::unordered_map<CellCoordinatesKey, OrthtreeCellRef>::iterator iter = cellCoordinatesMap_.find(key);
        if (iter != cellCoordinatesMap_.end()) {
            return iter->second;
        }
        return 0;
    }

    void GetCellCoordinatesForPosition(const float* pos, int16_t* coords) {
        assert(depth > 0, "cell size and depth of the orthtree must be set before use.");
        for (int dim = 0; dim < Dims; ++dim) {
            coords[dim] = (int16_t)(std::floor(pos[dim] / cellSize_));
        }
    }

    bool GetCoordinatesForCellRef(OrthtreeCellRef cellRef, int16_t* coords) {
        assert(depth > 0, "cell size and depth of the orthtree must be set before use.");
        if (cellRef > 0 && cellRef < cells_.size() && !cells_[cellRef].isEmpty) {
            std::memcpy(coords, cells_[cellRef].coords, Dims);
            return true;
        }
        return false;
    }

    void QueryNearestNeighbourCells(const float* point, std::function<float(OrthtreeCellRef, T&, float)> action) {
        assert(depth > 0, "cell size and depth of the orthtree must be set before use.");
        float minSqrDist = FLT_MAX;

        struct QueryCandidateNode {
            int nodeRef;
            uint16_t depth;
            int16_t bounds[Dims][2];
            float sqrDist;

            bool operator>(const QueryCandidateNode& other) const {
                return (sqrDist > other.sqrDist);
            }
        };

        const int16_t halfSize = size_ >> 1;
        std::vector<QueryCandidateNode> dfsStack;
        // Start with root node.
        QueryCandidateNode root;
        dfsStack.push_back(root);
        for (int dim = 0; dim < Dims; ++dim) {
            dfsStack[0].bounds[dim][0] = -halfSize;
            dfsStack[0].bounds[dim][1] = halfSize;
        }

        std::vector<QueryCandidateNode> childQueryCandidates;
        childQueryCandidates.reserve(ORTHTREE_CHILD_COUNT);
        while (!dfsStack.empty()) {
            QueryCandidateNode queryCandidate = dfsStack.back();
            dfsStack.pop_back();

            if (!(queryCandidate.sqrDist < minSqrDist)) {
                // It is not possible for this node to have contents
                // closer than minSqrDist to point.
                continue;
            }

            int firstChildRef = nodes_[queryCandidate.nodeRef].firstChild;
            if (firstChildRef < 0) {
                // This node has no children. Skip.
                continue;
            }

            if (queryCandidate.depth == (depth_ - 1)) {
                for (int i = 0; i < ORTHTREE_CHILD_COUNT; ++i) {
                    const OrthtreeCellRef cellRef = firstChildRef + i;
                    OrthtreeCell& cell = cells_[cellRef];
                    if (!cell.isEmpty) {
                        minSqrDist = std::min(minSqrDist, action(cellRef, cell.object, minSqrDist));
                    }
                }
            }
            else {
                for (int i = 0; i < ORTHTREE_CHILD_COUNT; ++i) {
                    QueryCandidateNode childQueryCandidate;
                    childQueryCandidate.nodeRef = firstChildRef + i;
                    childQueryCandidate.depth = queryCandidate.depth + 1;

                    // Calculate the bounding box for this child candidate node,
                    // as well as the square distance between the input point and
                    // the closest point on the bounding box.
                    for (int dim = 0; dim < Dims; ++dim) {
                        const bool greaterThanMid = (i % (2 << dim)) < (1 << dim);
                        const float mid = (queryCandidate.bounds[dim][0] + queryCandidate.bounds[dim][1]) >> 1;
                        childQueryCandidate.bounds[dim][!greaterThanMid] = mid;
                        childQueryCandidate.bounds[dim][greaterThanMid] = queryCandidate.bounds[dim][greaterThanMid];
                        const float pointCoordClamped = std::min(
                            std::max(childQueryCandidate.bounds[dim][0] * cellSize_, point[dim]),
                            childQueryCandidate.bounds[dim][1] * cellSize_
                        );
                        const float diff = point[dim] - pointCoordClamped;
                        childQueryCandidate.sqrDist += (diff * diff);
                    }

                    if (childQueryCandidate.sqrDist < minSqrDist) {
                        // Only consider this child node if it is less than minSqrDist to point.
                        childQueryCandidates.push_back(childQueryCandidate);
                    }
                }

                if (!childQueryCandidates.empty()) {
                    // Sort nodes by descending order of squared distance to point.
                    // This is the order we want to append them to the queue to ensure
                    // a depth-first search traversal.
                    std::sort(childQueryCandidates.begin(), childQueryCandidates.end(), std::greater<QueryCandidateNode>());
                    dfsStack.insert(dfsStack.end(), childQueryCandidates.begin(), childQueryCandidates.end());
                    childQueryCandidates.clear();
                }
            }
        }
    }

private:
    using OrthtreeInternalNodeRef = int32_t;
    using CellCoordinatesKey = uint64_t;

    // Non-leaf nodes.
    struct OrthtreeInternalNode {
        OrthtreeInternalNodeRef parent;

        // firstChild is the index of the first leaf child node.
        // The children are ordered in the following way:  
        // 
        //                          +Y
        //                           ^
        //                           |    -Z
        //         firstChild + 5    |    /    firstChild + 4
        //                           |   /
        //                           |  /
        //                           | /
        //     firstChild + 1        |/    firstChild + 0
        // -X <----------------------/-----------------------> +X
        //                          /|
        //         firstChild + 7  / |         firstChild + 6         
        //                        /  |
        //                       /   |
        //                      /    |
        //     firstChild + 3  /     |     firstChild + 2
        //                   +Z      |
        //                           v
        //                          -Y
        // 
        // If -1, this node has no children.
        int32_t firstChild;
    };

    // Leaf nodes.
    struct OrthtreeCell {
        bool isEmpty;
        OrthtreeInternalNodeRef parent;
        int16_t coords[Dims];
        T object;
    };

    std::vector<OrthtreeInternalNode> nodes_;
    std::queue<OrthtreeInternalNodeRef> freeNodeSlots_;
    std::vector<OrthtreeCell> cells_;
    std::queue<OrthtreeCellRef> freeCellSlots_;

    // Used for fast cell retrieval by coordinates.
    std::unordered_map<CellCoordinatesKey, OrthtreeCellRef> cellCoordinatesMap_;

    uint16_t depth_;
    int16_t size_;
    float cellSize_;

    OrthtreeInternalNodeRef AllocateChildrenNodes(const OrthtreeInternalNodeRef parentNodeRef) {
        OrthtreeInternalNodeRef freeNodeSlot;
        if (freeNodeSlots_.empty()) {
            freeNodeSlot = nodes_.size();
            for (int i = 0; i < ORTHTREE_CHILD_COUNT; ++i) {
                nodes_.push_back({ parentNodeRef, -1 });
            }
        }
        else {
            freeNodeSlot = freeNodeSlots_.front();
            for (int i = 0; i < ORTHTREE_CHILD_COUNT; ++i) {
                nodes_[freeNodeSlot + i] = { parentNodeRef, -1 };
            }
            freeNodeSlots_.pop();
        }
        return freeNodeSlot;
    }

    void DeallocateChildrenNodes(const OrthtreeInternalNodeRef parentNodeRef) {
        freeNodeSlots_.push(nodes_[parentNodeRef].firstChild);
    }

    OrthtreeCellRef AllocateChildrenCells(const OrthtreeInternalNodeRef parentNodeRef, const int16_t* mid) {
        OrthtreeCellRef freeCellSlot;
        if (freeCellSlots_.empty()) {
            freeCellSlot = cells_.size();
            cells_.resize(freeCellSlot + OCTREE_CHILD_COUNT);
        }
        else {
            freeCellSlot = freeCellSlots_.front();
            freeCellSlots_.pop();
        }

        for (int i = 0; i < ORTHTREE_CHILD_COUNT; ++i) {
            OrthtreeCell& cell = cells_[freeCellSlot + i];
            cell.isEmpty = true;
            cell.parent = parentNodeRef;
            for (int dim = 0; dim < Dims; ++dim) {
                cell.coords[dim] = mid[dim] + ORTHTREE_CHILD_CELL_OFFSETS[i][dim];
            }
        }

        return freeCellSlot;
    }

    void DeallocateChildrenCells(const OrthtreeInternalNodeRef parentNodeRef) {
        freeCellSlots_.push(nodes_[parentNodeRef].firstChild);
    }

    static CellCoordinatesKey keyForCellCoordinates(int16_t* coords) {
        uint64_t cellCoordsKey;
        char* tileCoordsKeyPtr = reinterpret_cast<char*>(&cellCoordsKey);
        const std::size_t coordSizeBytes = sizeof(coords[0]);
        for (int dim = 0; dim < Dims; ++dim) {
            std::memcpy(tileCoordsKeyPtr, &coords[dim], coordSizeBytes);
            tileCoordsKeyPtr += coordSizeBytes;
        }
        return cellCoordsKey;
    }
};

