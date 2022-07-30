#pragma once

#include <vector>
#include <queue>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <functional>
#include "DataStructures.hpp"

// Breadth First Search

#pragma region BFS
template <class DataType = void*>
struct BreadthFirstSearchNode
{
    using Node = BreadthFirstSearchNode<DataType>;

    std::vector<Node*> neighbors;
    bool enabled = true;
    DataType data;

    void connect(Node* node, bool bidirectional = true)
    {
        neighbors.push_back(node);
        if (bidirectional) node->connect(this, false);
    }
};

template <typename NodeDataType = void*>
class BreadthFirstSearch
{
    using Node = BreadthFirstSearchNode<NodeDataType>;

protected:
    std::vector<std::unique_ptr<Node>> nodes;

public:
    Node* addNode(std::vector<Node*> neighbors)
    {
        nodes.push_back(std::make_unique<Node>());
        for (auto& n : neighbors)
        { nodes.back()->connect(n); }
        return nodes.back().get();
    }

    Node* addNode()
    {
        nodes.push_back(std::make_unique<Node>());
        return nodes.back().get();
    }

    Node* addNode(std::initializer_list<Node*> nodeInitList)
    {
        nodes.push_back(std::make_unique<Node>());
        for (auto& n : nodeInitList)
        { nodes.back()->connect(n); }
        return nodes.back().get();
    }

    void removeNode(Node* nodeToRemove)
    {
        for (size_t i = 0; i < nodes.size(); i++)
        {
            if (nodes[i].get() == nodeToRemove)
            { nodes.erase(nodes.begin() + i); return; }
        }
    }

    const std::vector<std::unique_ptr<Node>>& getNodes() const { return nodes; }

    std::vector<Node*> findPath(Node* start, Node* goal, bool includeStart = false) const
    {
        bool goalFound = false;

        std::unordered_map<Node*, Node*> cameFrom;
        std::queue<Node*> frontier;
        frontier.push(start);

        while (!frontier.empty())
        {
            auto current = frontier.front(); frontier.pop();

            if (current == goal) { goalFound = true; break; }

            for (auto& neighbor : current->neighbors)
            {
                if (neighbor->enabled && !mapContains(cameFrom, neighbor))
                {
                    frontier.push(neighbor);
                    cameFrom[neighbor] = current;
                }
            }
        }

        if (goalFound)
        {
            std::vector<Node*> path;
            auto* current = goal;
            while (current != start)
            {
                path.push_back(current);
                current = cameFrom[current];
            }
            if (includeStart) path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }
        else
        { return std::vector<Node*>(); }

    }
};
#pragma endregion

// AStar
// Example:
// 
// Setup:
// 
// using Node       = AStar2DNode<Vector2i>;
// using Pathfinder = AStar2D<Vector2i>;
// 
// Pathfinder pathfinder;
// 
// Create a grid of nodes:
// 
// const Vector2i gridCount(32, 16);
// const std::vector<Vector2i> neighborsToAdd { Vector2i(-1, 0), Vector2i(-1, -1), Vector2i(0, -1), Vector2i(-1, 1) };
// for (size_t x = 0; x < gridCount.x; x++)
// {
//     for (size_t y = 0; y < gridCount.y; y++)
//     {
//         auto node = pathfinder.addNode();
//         Vector2i myPos = Vector2i(x, y);
//         node->position = myPos;
//
//         for (auto& posDiff : neighborsToAdd)
//         {
//             auto neighborNode = getNodeByPosition(myPos + posDiff);
//             if (neighborNode != nullptr)
//             { node->connect(neighborNode); }
//         }
//     }
// }
// 
// Calculate path:
// Node* startPoint = pathfinder.getNodes().front().get();
// Node* endPoint   = pathfinder.getNodes().back().get();
// std::vector<Node*> pointPath = pathfinder.findPath(startPoint, endPoint, true);
//
// You can use getNodes() to.. get the nodes.
// Then loop through them to draw them.
// Then loop through pointPath to draw your path.
//

template<typename T, typename priority_t>
struct PriorityQueue
{
    typedef std::pair<priority_t, T> PQElement;
    std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> elements;

    inline bool empty() const { return elements.empty(); }

    inline void put(T item, priority_t priority) { elements.emplace(priority, item); }

    T get()
    {
        T best_item = elements.top().second;
        elements.pop();
        return best_item;
    }
};

template <typename KeyType, typename ValueType>
bool mapContains(std::unordered_map<KeyType, ValueType>& map, KeyType key)
{
    return (map.find(key) != map.end());
}

struct AStar2DNode
{
    // DO NOT CHANGE THESE THEY ARE READ-ONLY
    int x, y;

    std::vector<AStar2DNode*> connections;
    float moveCost = 1.f;
    bool  enabled = true;

    AStar2DNode(const int xv, const int yv, const int neighborReserve = 8) : x{ xv }, y{ yv }
    {
        connections.reserve(neighborReserve);
    }

    ~AStar2DNode()
    {
        clearConnections();
    }

    inline Vector2i getPosition() const noexcept
    { return {x, y}; }

    void connect(AStar2DNode* const node, const bool bidirectional = true)
    {
        if (!hasConnection(node)) { connections.push_back(node); }
        if (bidirectional) { node->connect(this, false); }
    }

    void disconnect(AStar2DNode* const node)
    {
        for (size_t i = 0; i < connections.size(); i++)
        {
            if (connections[i] == node)
            {
                connections.erase(connections.begin() + i);
                return;
            }
        }
    }

    void clearConnections()
    {
        for (auto& n : connections) { n->disconnect(this); }
        connections.clear();
    }

    bool hasConnectionAt(const int x, const int y)
    {
        for (auto& c : connections)
        { if (c->x == x && c->y == y) return true; }
        return false;
    }

    bool hasConnection(const AStar2DNode* const node) const
    {
        return std::find(connections.begin(), connections.end(), node) != connections.end();
    }
};

class AStar2D
{
public:
    friend struct AStar2DNode;
    using Node = AStar2DNode;
    using ValidPositionCallback = std::function<bool(const Vector2i&)>;

    // TODO: use hash map
    std::list<Node> nodes;

    struct FrontierComparator
    { bool operator()(const Node* a, const Node* b) { return a->moveCost < b->moveCost; } };

    Node* addNode(const int x, const int y, const int neighborReserve = 8)
    {
        auto nodeToOverwrite = getNodeByPosition(x, y);
        if (nodeToOverwrite != nullptr) removeNode(nodeToOverwrite);
        nodes.emplace_back(x, y, neighborReserve);
        return &nodes.back();
    }

    void removeNode(Node* const nodeToRemove)
    {
        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            if (&*it == nodeToRemove)
            {
                nodes.erase(it);
                return;
            }
        }
    }

    void removeNodeByPosition(const int x, const int y)
    {
        auto n = getNodeByPosition(x, y);
        if (n != nullptr) removeNode(n);
    }
    void removeNodeByPosition(const Vector2i& v) { removeNodeByPosition(v.x, v.y); }

    // Deletes all nodes
    void clear() { nodes.clear(); }

    float heuristic(const Node* const a, const Node* const b) const
    {
        auto apos = a->getPosition(); auto bpos = b->getPosition();
        return abs(apos.x - bpos.x) + abs(apos.y - bpos.y);
    };

    // Returns nullptr if there is no node at position
    Node* getNodeByPosition(const int x, const int y)
    {
        for (Node& node : nodes)
        {
            auto pos = node.getPosition();
            if (pos.x == x && pos.y == y) { return &node; }
        }
        return nullptr;
    }
    Node* getNodeByPosition(const Vector2i& v) { return getNodeByPosition(v.x, v.y); }

    // TODO: finish maybe
    //Node* getNodeClosestToPosition(const Vector2i& v, size_t maxDistance = SIZE_MAX)
    //{
    //    auto* start = getNodeByPosition(v.x, v.y);
    //    if (start != nullptr)
    //    { return start; }
    //
    //    for (int d = 0; d < maxDistance; d++)
    //    {
    //        for (int x = xs-d; x < xs+d+1; x++)
    //        {
    //            // Point to check: (x, ys - d) and (x, ys + d) 
    //            if (CheckPoint(x, ys - d) == true)
    //            {
    //                return (x, ys - d);
    //            }
    //
    //            if (CheckPoint(x, ys + d) == true)
    //            {
    //                return (x, ys - d);
    //            }
    //        }
    //
    //        for (int y = ys-d+1; y < ys+d; y++)
    //        {
    //            // Point to check = (xs - d, y) and (xs + d, y) 
    //            if (CheckPoint(x, ys - d) == true)
    //            {
    //                return (xs - d, y);
    //            }
    //
    //            if (CheckPoint(x, ys + d) == true)
    //            {
    //                return (xs - d, y);
    //            }
    //        }
    //    }
    //}

    // Will return an empty vector if there is no path.
    const std::vector<Node*> findPath(Node* start, Node* goal, const bool includeStart = false) const
    {
        bool goalFound = false;
        std::unordered_map<Node*, Node*> cameFrom;
        std::unordered_map<Node*, float> costSoFar;
        auto frontier = PriorityQueue<Node*, float>();

        cameFrom[start] = nullptr;
        costSoFar[start] = 0.f;
        frontier.put(start, 0);

        while (!frontier.empty())
        {
            auto current = frontier.get();

            if (current == goal) { goalFound = true; break; }

            for (auto& nextNode : current->connections)
            {
                if (nextNode->enabled == false) { continue; }

                auto newCost = costSoFar[current] + nextNode->moveCost;
                if (!mapContains(costSoFar, nextNode) || newCost < costSoFar[nextNode])
                {
                    costSoFar[nextNode] = newCost;
                    frontier.put(nextNode, newCost + heuristic(goal, nextNode));
                    cameFrom[nextNode] = current;
                }
            }
        }

        if (goalFound)
        {
            std::vector<Node*> path;
            auto* current = goal;
            while (current != start)
            {
                path.push_back(current);
                current = cameFrom[current];
            }
            if (includeStart) path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }
        else
        { return std::vector<Node*>(); }
    }
};
