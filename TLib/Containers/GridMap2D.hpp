
#pragma once
#include <TLib/DataStructures.hpp>
#include <TLib/Containers/Vector.hpp>
#include <TLib/Containers/Array.hpp>
#include <TLib/Containers/UnorderedSet.hpp>
#include <TLib/Containers/Queue.hpp>
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/Containers/Pair.hpp>
#include <TLib/Logging.hpp>
#include <TLib/thirdparty/multiarray/array.h>

struct GridMap2DGrid
{
    bool  passable = true;
    float cost = 1.f;

    GridMap2DGrid(bool passable, float cost) : passable{ passable }, cost{ cost } { }
    GridMap2DGrid() = default;
};

struct GridMap2D
{
protected:
    using Grid      = GridMap2DGrid;
    using Array2D   = nda::shape<nda::dim<>, nda::dim<>>;
    using GridArray = nda::array<Grid, Array2D>;

    const Array<Vector2i, 8> dirs = {
        Vector2i{1,  0}, Vector2i{-1, 0},
        Vector2i{0, -1}, Vector2i{ 0, 1},

        // Diagonal directions
        Vector2i{ 1, -1}, Vector2i{  1,  1},
        Vector2i{-1,  1}, Vector2i{ -1, -1} // TODO: Make diagonals optional
    };

    GridArray grids;
    Vector2i  size;

    bool passable(const Vector2i& pos) const
    { return getGridAt(pos).passable; }

    Vector<Vector2i> neighbors(const Vector2i& pos) const
    {
        Vector<Vector2i> results;
        results.reserve(dirs.size());
        for (Vector2i dir : dirs)
        {
            Vector2i next{ pos.x + dir.x, pos.y + dir.y };
            if (inBounds(next) && passable(next))
            { results.push_back(next); }
        }

        return results;
    }

    float cost(const Vector2i& fromPos, const Grid& fromGrid, const Vector2i& toPos, const Grid& toGrid) const
    {
        bool diagonal = (fromPos.x != toPos.x && fromPos.y != toPos.y);
        return toGrid.cost * (diagonal ? diagonalCost : 1.f);
    }

    float heuristic(const Vector2i& node, const Vector2i& goal)
    {
        return max(fabs((float)goal.x - node.x), fabs((float)goal.y - node.y));
    }

    template <typename T>
    struct GridPriorityQueueCompare
    {
        constexpr bool operator()(const T& a, const T& b) const
        { return a.first > b.first; }
    };

    template<typename T, typename priority_t>
    struct GridPriorityQueue
    {
        using PQElement = Pair<priority_t, T>;

        PriorityQueue<PQElement, DefaultQueueContainer<PQElement>, GridPriorityQueueCompare<PQElement>> elements;

        void clear() { elements.get_container().clear(); }

        inline bool empty() const { return elements.empty(); }

        inline void put(T item, priority_t priority)
        { elements.emplace(priority, item); }

        T pop()
        {
            T bestItem = elements.top().second;
            elements.pop();
            return bestItem;
        }
    };

public:

    float diagonalCost = 1.001f; // Read/Write; I recommend 1.001 or sqrt(2)
    bool  includeStart = false; // Read/Write; Should the start variable be included in computePath return.

    bool inBounds(const Vector2i& pos) const
    {
        return 0 <= pos.x && pos.x < width()
            && 0 <= pos.y && pos.y < height();
    }

    void setSize(int width = 0, int height = 0) { setSize({ width, height }); }

    void setSize(const Vector2i& newSize)
    {
        size = newSize;
        grids.reshape(Array2D(width(), height()));
    }

          Grid& getGridAt(const Vector2i& pos)       { ASSERT(inBounds(pos)); return grids(pos.x, pos.y); }
          Grid& getGridAt(int x, int y)              { return getGridAt({ x, y }); }
    const Grid& getGridAt(const Vector2i& pos) const { ASSERT(inBounds(pos)); return grids(pos.x, pos.y); }
    const Grid& getGridAt(int x, int y)        const { return getGridAt({ x, y }); }

    int      width()   const { return size.x; }
    int      height()  const { return size.y; }
    Vector2i getSize() const { return size;   }

    GridPriorityQueue<Vector2i, float> frontier; // TODO: make class scope
    UnorderedMap<Vector2i, Vector2i>   internalCameFrom;  // TODO: make class scope
    UnorderedMap<Vector2i, float>      internalCostSoFar; // TODO: make class scope

    Vector<Vector2i> computePath(
        const Vector2i& start,
        const Vector2i& goal,
        UnorderedMap<Vector2i, Vector2i>* cameFromPtr  = nullptr, // For debug
        UnorderedMap<Vector2i, float>*    costSoFarPtr = nullptr) // For debug
    {
        if (!inBounds(start)) { return Vector<Vector2i>(); }
        if (!inBounds(goal))  { return Vector<Vector2i>(); }

        if (!cameFromPtr)  { cameFromPtr  = &internalCameFrom; }
        if (!costSoFarPtr) { costSoFarPtr = &internalCostSoFar; }

        UnorderedMap<Vector2i, Vector2i>& cameFrom  = *cameFromPtr;
        UnorderedMap<Vector2i, float>&    costSoFar = *costSoFarPtr;

        // If the goal is impassable, we want to try and return a path leading next to it.
        // So make it temporarily passable and pop it before returning.
        // TODO: Make this a setting
        // TODO: Make this work for further distances
        Grid& goalGrid = getGridAt(goal);
        bool goalIsPassable = goalGrid.passable;
        goalGrid.passable = true;

        frontier.clear();
        frontier.put(start, 0.f);

        cameFrom.clear();
        costSoFar.clear();
        cameFrom[start] = start;
        costSoFar[start] = 0.f;

        while (!frontier.empty())
        {
            Vector2i current = frontier.pop();

            if (current == goal) { break; }

            for (Vector2i next : neighbors(current))
            {
                float newCost = costSoFar[current] + cost(current, getGridAt(current), next, getGridAt(next));
                if (costSoFar.find(next) == costSoFar.end() || newCost < costSoFar[next])
                {
                    costSoFar[next] = newCost;
                    float priority  = newCost + heuristic(next, goal);
                    frontier.put(next, priority);
                    cameFrom[next]  = current;
                }
            }
        }

        goalGrid.passable = goalIsPassable;

        // Reconstruct path
        Vector<Vector2i> path;
        Vector2i current = goal;

        if (cameFrom.find(goal) == cameFrom.end())
        { return path; } // no path can be found

        while (current != start)
        {
            path.push_back(current);
            current = cameFrom[current];
        }

        if (includeStart)
        { path.push_back(start); }

        std::reverse(path.begin(), path.end());

        if (!goalIsPassable && path.size() > 0)
        { path.pop_back(); }

        return path;
    }
};
