#pragma once

#include <vector>
#include <queue>
#include <set>
#include <unordered_set>
#include <unordered_map>

// Breadth First Search

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
                if (neighbor->enabled && !cameFrom.contains(neighbor))
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

// AStar
// It's not great but it works.
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

// UNTESTED
template <typename Vector3DType>
struct AStar3DNode
{
    std::vector<AStar3DNode*> neighbors;
    float moveCost = 1.f;
    bool  enabled = true;
    Vector3DType position;

    void connect(AStar3DNode* node, bool bidirectional = true)
    {
        neighbors.push_back(node);
        if (bidirectional) node->connect(this, false);
    }
};

// UNTESTED
template <typename Vector3DType>
class AStar3D
{
    using Node = AStar3DNode<Vector3DType>;

protected:
    std::vector<std::unique_ptr<Node>> nodes;

public:
    struct FrontierComparator
    { bool operator()(const Node* a, const Node* b) { return a->moveCost < b->moveCost; } };

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

    float heuristic(const Node* a, const Node* b)
    { return abs(a->position.x - b->position.x) + abs(a->position.y - b->position.y) + abs(a->position.z - b->position.z); };

    // Will return an empty vector if there is no path.
    std::vector<Node*> findPath(Node* start, Node* goal, bool includeStart = false) const
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

            for (auto& nextNode : current->neighbors)
            {
                if (nextNode->enabled == false) continue;

                auto newCost = costSoFar[current] + nextNode->moveCost;
                if (!costSoFar.contains(nextNode) || newCost < costSoFar[nextNode])
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

template <typename Vector2DType>
struct AStar2DNode
{
    std::vector<AStar2DNode*> neighbors;
    float moveCost = 1.f;
    bool  enabled = true;
    Vector2DType position;

    void connect(AStar2DNode* node, bool bidirectional = true)
    {
        neighbors.push_back(node);
        if (bidirectional) node->connect(this, false);
    }
};

template <typename Vector2DType>
class AStar2D
{
    using Node = AStar2DNode<Vector2DType>;

protected:
    std::vector<std::unique_ptr<Node>> nodes;

public:
    struct FrontierComparator
    { bool operator()(const Node* a, const Node* b) { return a->moveCost < b->moveCost; } };

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

    float heuristic(const Node* a, const Node* b) const { return abs(a->position.x - b->position.x) + abs(a->position.y - b->position.y); };

    // Returns nullptr if there is no node at position
    Node* getNodeByPosition(const Vector2DType& position) const
    {
        for (auto& node : nodes)
        { if (node->position == position) return node.get(); }
        return nullptr;
    }

    // Will return an empty vector if there is no path.
    std::vector<Node*> findPath(Node* start, Node* goal, bool includeStart = false) const
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

            for (auto& nextNode : current->neighbors)
            {
                if (nextNode->enabled == false) continue;

                auto newCost = costSoFar[current] + nextNode->moveCost;
                if (!costSoFar.contains(nextNode) || newCost < costSoFar[nextNode])
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