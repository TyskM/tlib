# tlib
Some headers I use a lot.

# Pathfind.hpp AStar example
Setup:
```cpp
using Node       = AStar2DNode<Vector2i>; // Use your 2D vector class
using Pathfinder = AStar2D<Vector2i>;

Pathfinder pathfinder;
```

Create a grid of nodes and connect them:
```cpp
const Vector2i gridCount(32, 16);
const std::vector<Vector2i> neighborsToAdd { Vector2i(-1, 0), Vector2i(-1, -1), Vector2i(0, -1), Vector2i(-1, 1) };
for (size_t x = 0; x < gridCount.x; x++)
{
    for (size_t y = 0; y < gridCount.y; y++)
    {
        auto node = pathfinder.addNode();
        Vector2i myPos = Vector2i(x, y);
        node->position = myPos;

        for (auto& posDiff : neighborsToAdd)
        {
            auto neighborNode = getNodeByPosition(myPos + posDiff);
            if (neighborNode != nullptr)
            { node->connect(neighborNode); }
        }
    }
}
```

Calculate path:
```cpp
Node* startPoint = pathfinder.getNodes().front().get();
Node* endPoint   = pathfinder.getNodes().back().get();
std::vector<Node*> pointPath = pathfinder.findPath(startPoint, endPoint, true);
//\\ The boolean is for including the start point in the returned list. It is false by default.
```
You can use getNodes() to.. get the nodes,  
Then loop through them to draw them.  
Then loop through pointPath to draw your path. 
