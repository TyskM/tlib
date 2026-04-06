
#pragma once
#include <TLib/Types/Types.hpp>
#include <TLib/Containers/Vector.hpp>
#include <TLib/Containers/Array.hpp>
#include <TLib/Containers/UnorderedSet.hpp>
#include <TLib/Containers/Queue.hpp>
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/Containers/Pair.hpp>
#include <TLib/Logging.hpp>
#include <TLib/Containers/GridMap2D.hpp>
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

enum class AStar2DPathStatus
{
    Success,
    PartialPath,
    NoPath
};

struct AStar2DPath
{
    AStar2DPathStatus status = AStar2DPathStatus::NoPath;
    Vector<Vector2i>  nodes;
};

template <typename GridType = AStar2DGrid>
struct AStar2D
{
    static_assert(std::is_base_of<AStar2DGrid, GridType>::value, "GridType must derive from AStar2DGrid");
    static_assert(std::is_default_constructible<GridType>(),     "GridType must be default constructible");

protected:
    using Grid = GridType;

    static constexpr Array<Vector2i, 8> dirs = {
        Vector2i{1,  0}, Vector2i{-1, 0},
        Vector2i{0, -1}, Vector2i{ 0, 1},

        // Diagonal directions
        Vector2i{ 1, -1}, Vector2i{  1,  1},
        Vector2i{-1,  1}, Vector2i{ -1, -1}
    };

    GridMap2D<GridType> grids;

    Vector<Vector2i> neighbors(const Vector2i& pos, bool diagonals) const
    {
        Vector<Vector2i> results;
        int32_t dirCount = diagonals ? 8 : 4;
        results.reserve(dirCount);
        for(size_t i = 0; i < dirCount; i++)
        {
            auto& dir = dirs[i];
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

    AStar2D()                     { resize(10, 10); }
    AStar2D(const Vector2i& size) { resize(size);   }
    AStar2D(int32_t x, int32_t y) { resize(x, y);   }

    bool passable(const Vector2i& pos)  const { return at(pos).passable; }
    bool passable(int32_t x, int32_t y) const { return passable({x, y}); }

    bool inBounds(const Vector2i& pos) const
    { return grids.inBounds(pos); }

    void resize(int32_t width, int32_t height)
    { resize({ width, height }); }

    void resize(const Vector2i& newSize)
    { grids.resize(newSize); }

    void clear()
    { grids.clear(); }

    void clear(const GridType& grid)
    { grids.clear(grid); }

          Grid& at(const Vector2i& pos)        { return grids.at(pos.x, pos.y); }
          Grid& at(int32_t x, int32_t y)       { return at({ x, y }); }
    const Grid& at(const Vector2i& pos)  const { return grids.at(pos.x, pos.y); }
    const Grid& at(int32_t x, int32_t y) const { return at({ x, y }); }

    int32_t  width()  const { return grids.width();  }
    int32_t  height() const { return grids.height(); }
    Vector2i size()   const { return grids.size();   }

    mutable GridPriorityQueue<Vector2i, float> frontier;
    
    using CameFromMap  = UnorderedMap<Vector2i, Vector2i>;
    using CostSoFarMap = UnorderedMap<Vector2i, float>;

    mutable CameFromMap  internalCameFrom;
    mutable CostSoFarMap internalCostSoFar;

    mutable CameFromMap*  cameFromPtr  = nullptr;
    mutable CostSoFarMap* costSoFarPtr = nullptr;

    CameFromMap& getCameFromMap() const
    {
        if (cameFromPtr) { return *cameFromPtr; }
        return internalCameFrom;
    }

    CostSoFarMap& getCostSoFarMap() const
    {
        if (costSoFarPtr) { return *costSoFarPtr; }
        return internalCostSoFar;
    }

    AStar2DPath computePath(
        const Vector2i& start,
        const Vector2i& goal,
        const float     diagonalCost = FLT_MAX) const
    {
        const bool allowDiagonals = diagonalCost != FLT_MAX;

        if (start == goal)
        {
            AStar2DPath p;
            p.status = AStar2DPathStatus::Success;
            return p;
        }

        if (!inBounds(start)) { return AStar2DPath{AStar2DPathStatus::NoPath}; }
        if (!inBounds(goal))  { return AStar2DPath{AStar2DPathStatus::NoPath}; }

        UnorderedMap<Vector2i, Vector2i>& cameFrom  = getCameFromMap();
        UnorderedMap<Vector2i, float>&    costSoFar = getCostSoFarMap();

        // If the goal is impassable, we want to try and return a path leading next to it.
        // So make it temporarily passable and pop it before returning.
        // DONE: Make this a setting
        // TODO: Make this work for further distances
        const Grid& goalGridConst = at(goal);
        // HACK: Changes are reverted at the end of the function, dunno if there's a better way.
        Grid& goalGrid      = const_cast<Grid&>(goalGridConst);
        bool goalIsPassable = goalGrid.passable;
        goalGrid.passable   = true;

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

            for (Vector2i next : neighbors(current, allowDiagonals))
            {
                float newCost = costSoFar[current] + cost(current, at(current), next, at(next), diagonalCost);
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

        // no path can be found
        if (cameFrom.find(goal) == cameFrom.end())
        { return AStar2DPath{AStar2DPathStatus::NoPath}; }

        AStar2DPath path{};
        path.status = AStar2DPathStatus::Success;

        Vector2i current = goal;
        while (current != start)
        {
            path.nodes.push_back(current);
            current = cameFrom[current];
        }

        std::reverse(path.nodes.begin(), path.nodes.end());

        if (!goalIsPassable)
        {
            path.nodes.pop_back();
            path.status = AStar2DPathStatus::PartialPath;
        }

        return path;
    }

    Vector<Vector2i> line(const Vector2i& start, const Vector2i& end) const
    {
        Vector<Vector2i> points;
        Vector2f startf(start);
        Vector2f endf(end);

        float N = diagDist(startf, endf);
        for (int32_t step = 0; step <= N; step++)
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
        for (int32_t step = 0; step <= N; step++)
        {
            float t = (N == 0) ? 0.0 : (float)step / N;
            Vector2i nextPoint = Vector2i(v2flerp(startf, endf, t).rounded());

            if (grids) { grids->push_back(nextPoint); }

            if (!inBounds(nextPoint) || !at(nextPoint).passable)
            { return AStar2DRaycastResult(true, nextPoint); }
        }
        return AStar2DRaycastResult(false, end);
    }
};
