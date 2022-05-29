# tlib
Some headers I use a lot.

# Pathfind.hpp AStar example

![AStar example](https://imgur.com/IKYpU6G.gif)

Setup:
```cpp
using Node       = AStar2DNode;
using Pathfinder = AStar2D;

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
        auto node = pathfinder.addNode(x, y);

        for (auto& posDiff : neighborsToAdd)
        {
            auto neighborNode = pathfinder.getNodeByPosition(x + posDiff.x, y + posDiff.y);
            if (neighborNode != nullptr)
            { node->connect(neighborNode); }
        }
    }
}
```

Calculate path:
```cpp
Node* startPoint = &pathfinder.nodes.front(); // Get any two points
Node* endPoint   = &pathfinder.nodes.back();
std::vector<Node*> pointPath = pathfinder.findPath(startPoint, endPoint, true); // Then call findPath
//\\ The boolean is for including the start point in the returned list. It is false by default.
```
You can loop through pathfinder.nodes to draw the nodes. 
Then loop through pointPath to draw your path. 
