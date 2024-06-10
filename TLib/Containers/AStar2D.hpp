
#pragma once
#include <TLib/Types/Types.hpp>
#include <TLib/Containers/Vector.hpp>
#include <TLib/Containers/Array.hpp>
#include <TLib/Containers/UnorderedSet.hpp>
#include <TLib/Containers/Queue.hpp>
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/Containers/Pair.hpp>
#include <TLib/Logging.hpp>
#include <TLib/thirdparty/multiarray/array.h>

struct AStar2DGrid
{
    bool  passable = true;
    float moveCost = 1.f;

    AStar2DGrid(bool passable, float cost) : passable{ passable }, moveCost{ cost } { }
    AStar2DGrid() = default;
};

struct AStar2DRaycastResult
{
    bool     hit = false;
    Vector2i pos;

    AStar2DRaycastResult(bool hit, const Vector2i& pos) : hit{ hit }, pos{ pos } { }
    AStar2DRaycastResult() = default;
};

// TODO: Rename to AStarGridMap2D
template <typename GridType = AStar2DGrid>
struct AStar2D
{
    static_assert(std::is_base_of<AStar2DGrid, GridType>::value, "GridType must derive from AStar2DGrid");
    static_assert(std::is_default_constructible<GridType>(),     "GridType must be default constructible");

protected:

    // TODO: Line of sight support

    using Grid      = GridType;
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

    float cost(const Vector2i& fromPos, const Grid& fromGrid, const Vector2i& toPos, const Grid& toGrid, float diagonalCost) const
    {
        bool diagonal = (fromPos.x != toPos.x && fromPos.y != toPos.y);
        return toGrid.moveCost * (diagonal ? diagonalCost : 1.f);
    }

    float heuristic(const Vector2i& node, const Vector2i& goal) const
    {
        return max(fabs((float)goal.x - node.x), fabs((float)goal.y - node.y));
    }

    float diagDist(const Vector2f& a, const Vector2f& b) const
    {
        float dx = b.x - a.x, dy = b.y - a.y;
        return std::max(std::abs(dx), std::abs(dy));
    };

    Vector2f v2flerp(const Vector2f& a, const Vector2f& b, float t) const
    {
        return a * (1.f - t) + (b * t);
    };

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

    AStar2D(const Vector2i& size) { setSize(size); }
    AStar2D(int x, int y)         { setSize(x, y); }

    bool passable(const Vector2i& pos) const { return getGridAt(pos).passable; }
    bool passable(int x, int y)        const { return passable({x, y}); }

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

    void clear()
    {
        for (size_t x = 0; x < grids.width();  x++) {
        for (size_t y = 0; y < grids.height(); y++)
        { grids(x, y) = GridType(); }}
    }

    void clear(const GridType& grid)
    {
        for (size_t x = 0; x < grids.width();  x++) {
        for (size_t y = 0; y < grids.height(); y++)
        { grids(x, y) = grid; }}
    }

          Grid& getGridAt(const Vector2i& pos)       { ASSERT(inBounds(pos)); return grids(pos.x, pos.y); }
          Grid& getGridAt(int x, int y)              { return getGridAt({ x, y }); }
    const Grid& getGridAt(const Vector2i& pos) const { ASSERT(inBounds(pos)); return grids(pos.x, pos.y); }
    const Grid& getGridAt(int x, int y)        const { return getGridAt({ x, y }); }

          Grid& at(const Vector2i& pos)       { ASSERT(inBounds(pos)); return grids(pos.x, pos.y); }
          Grid& at(int x, int y)              { return at({ x, y }); }
    const Grid& at(const Vector2i& pos) const { ASSERT(inBounds(pos)); return grids(pos.x, pos.y); }
    const Grid& at(int x, int y)        const { return at({ x, y }); }

    int      width()   const { return size.x; }
    int      height()  const { return size.y; }
    Vector2i getSize() const { return size;   }

    mutable GridPriorityQueue<Vector2i, float> frontier;
    mutable UnorderedMap<Vector2i, Vector2i>   internalCameFrom;
    mutable UnorderedMap<Vector2i, float>      internalCostSoFar;

    Vector<Vector2i> computePath(
        const Vector2i& start,
        const Vector2i& goal,
        const bool      includeStart = false,
        const float     diagonalCost = 1.001f,
        UnorderedMap<Vector2i, Vector2i>* cameFromPtr  = nullptr, // For debug
        UnorderedMap<Vector2i, float>*    costSoFarPtr = nullptr) // For debug
        const
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
        const Grid& goalGridConst = getGridAt(goal);
        // HACK: Changes are reverted at the end of the function, dunno if there's a better way.
        Grid& goalGrid       = const_cast<Grid&>(goalGridConst);
        bool goalIsPassable  = goalGrid.passable;
        goalGrid.passable    = true;

        frontier.clear();
        frontier.put(start, 0.f);

        cameFrom.clear();
        costSoFar.clear();
        cameFrom [start] = start;
        costSoFar[start] = 0.f;

        while (!frontier.empty())
        {
            Vector2i current = frontier.pop();

            if (current == goal) { break; }

            for (Vector2i next : neighbors(current))
            {
                float newCost = costSoFar[current] + cost(current, getGridAt(current), next, getGridAt(next), diagonalCost);
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

    Vector<Vector2i> line(const Vector2i& start, const Vector2i& end) const
    {
        Vector<Vector2i> points;
        Vector2f startf(start);
        Vector2f endf(end);

        float N = diagDist(startf, endf);
        for (int step = 0; step <= N; step++)
        {
            float t = (N == 0) ? 0.0 : (float)step / N;
            points.push_back(Vector2i(v2flerp(startf, endf, t).rounded()));
        }
        return points;
    }

    AStar2DRaycastResult raycast(const Vector2i& start, const Vector2i& end, Vector<Vector2i>* grids = nullptr) const
    {
        Vector2f startf(start);
        Vector2f endf(end);

        if (grids)
        {
            grids->clear();
            grids->reserve( start.distanceTo(end) + size_t(1) );
        }

        float N = diagDist(startf, endf);
        for (int step = 0; step <= N; step++)
        {
            float t = (N == 0) ? 0.0 : (float)step / N;
            Vector2i nextPoint = Vector2i(v2flerp(startf, endf, t).rounded());

            if (grids) { grids->push_back(nextPoint); }

            if (!inBounds(nextPoint) || !getGridAt(nextPoint).passable)
            { return AStar2DRaycastResult(true, nextPoint); }
        }
        return AStar2DRaycastResult(false, end);
    }

    // TODO: Line of sight
    // https://www.roguebasin.com/index.php/Permissive_Field_of_View
};
