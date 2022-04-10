
#define SFML
#include <iostream>
#include "DataStructures.hpp"
#include "Pathfind.hpp"
#include "StringHelpers.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

#define ASSERT(x) if( !(x) ) { std::cout << "Assertion in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ << std::endl; abort(); }

namespace BreadthFirstSearchExample
{
    struct Node2DData
    {
        Vector2i position;
    };

    BreadthFirstSearch<Node2DData> bfs;
    using Node = BreadthFirstSearchNode<Node2DData>;

    Node* getNodeByPosition(Vector2i position)
    {
        for (auto& node : bfs.getNodes())
        {
            if (node->data.position == position)
                return node.get();
        }
        return nullptr;
    }

    void drawNodes(sf::RenderTarget& target, const BreadthFirstSearch<Node2DData>& bfs, const Node* start, const Node* end, std::vector<Node*> path, const float gridSize)
    {
        sf::RectangleShape nodeShape;
        nodeShape.setSize(Vector2f(15, 15));
        nodeShape.setOrigin(-nodeShape.getSize().x / 2, -nodeShape.getSize().y / 2);

        const auto& nodes = bfs.getNodes();

        for (auto& node : nodes)
        {
            // Draw connections
            if (node->enabled)
            {
                const auto& myPos = (Vector2f(node->data.position) * gridSize) + (gridSize / 2);
                for (auto& n : node->neighbors)
                {
                    if (n->enabled == false) continue;
                    const auto& nPos = (Vector2f(n->data.position) * gridSize) + (gridSize / 2);

                    sf::Vertex line[] =
                    { sf::Vertex(myPos, sf::Color(255, 255, 255, 20)), sf::Vertex(nPos, sf::Color(255, 255, 255, 20)) };
                    target.draw(line, 2, sf::Lines);
                }
            }

            // Draw path
            if (!path.empty())
            {
                auto p1 = path.front();

                for (auto& point : path)
                {
                    if (point == p1) continue;
                    else
                    {
                        sf::Vertex line[] =
                        {
                            sf::Vertex(Vector2f(p1->data.position)    * gridSize + (gridSize / 2), sf::Color::Yellow),
                            sf::Vertex(Vector2f(point->data.position) * gridSize + (gridSize / 2), sf::Color::Yellow)
                        };
                        target.draw(line, 2, sf::Lines);

                        p1 = point;
                    }
                }
            }
        }

        // Draw the bodies last, so their on top
        for (auto& node : nodes)
        {
            // Node body
            nodeShape.setPosition(node->data.position.x * gridSize, node->data.position.y * gridSize);

            if (node->enabled == false)
            { nodeShape.setFillColor(sf::Color(25, 25, 25)); }
            else if (node.get() == start)
            { nodeShape.setFillColor(sf::Color::Green); }
            else if (node.get() == end)
            { nodeShape.setFillColor(sf::Color::Red); }
            else
            { nodeShape.setFillColor(sf::Color(75, 75, 75)); }

            target.draw(nodeShape);
        }
    }

    void start()
    {
        const Vector2i gridCount(32, 16);
        const std::vector<Vector2i> neighborsToAdd { Vector2i(-1, 0), Vector2i(-1, -1), Vector2i(0, -1), Vector2i(-1, 1) };

        for (size_t x = 0; x < gridCount.x; x++)
        {
            for (size_t y = 0; y < gridCount.y; y++)
            {
                auto node = bfs.addNode();
                Vector2i myPos = Vector2i(x, y);
                node->data.position = myPos;

                for (auto& posDiff : neighborsToAdd)
                {
                    auto neighborNode = getNodeByPosition(myPos + posDiff);
                    if (neighborNode != nullptr)
                    { node->connect(neighborNode); }
                }
            }
        }

        const float gridSize = 32; // Size of each grid in pixels
        Node* startPoint = bfs.getNodes().front().get();
        Node* endPoint   = bfs.getNodes().back().get();
        std::vector<Node*> pointPath;
        bool needToRecalcPath = true;

        sf::RenderWindow window(sf::VideoMode(1280, 720), "Test");
        while (window.isOpen())
        {
            // Input
            sf::Event e;
            while (window.pollEvent(e))
            {
                if (e.type == sf::Event::Closed) window.close();

                if (e.type == sf::Event::MouseButtonPressed)
                {
                    Vector2i clickedPos = Vector2i(sf::Mouse::getPosition(window)) / gridSize;
                    auto cp = getNodeByPosition(clickedPos);

                    if (cp != nullptr && e.mouseButton.button == sf::Mouse::Left)
                    {
                        if   (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { cp->enabled = !cp->enabled; }
                        else { startPoint = cp; }
                        needToRecalcPath = true;
                    }
                    else if (cp != nullptr && e.mouseButton.button == sf::Mouse::Right)
                    {
                        endPoint = cp;
                        needToRecalcPath = true;
                    }
                }
            }

            // Update

            if (needToRecalcPath)
            {
                std::cout << "Path changed, recalculating... \n";
                needToRecalcPath = false;
                pointPath = bfs.findPath(startPoint, endPoint, true);

                if (pointPath.empty()) std::cout << "No path found!\n";
                else for (auto& point : pointPath) { std::cout << point->data.position.toString() << std::endl; }

                std::cout << "Recalc complete. \n";
            }

            // Draw
            window.clear();

            drawNodes(window, bfs, startPoint, endPoint, pointPath, gridSize);

            window.display();
        }
    }
}

namespace AStarExample
{
    sf::Font defaultFont;

    using Node       = AStar2DNode<Vector2i>;
    using Pathfinder = AStar2D<Vector2i>;
    
    Pathfinder pathfinder;

    void drawNodes(sf::RenderTarget& target, const Pathfinder& pathfinder, const Node* start, const Node* end, std::vector<Node*> path, const float gridSize)
    {
        sf::RectangleShape nodeShape;
        nodeShape.setSize(Vector2f(15, 15));
        nodeShape.setOrigin(-nodeShape.getSize().x / 2, -nodeShape.getSize().y / 2);

        const auto& nodes = pathfinder.getNodes();

        for (auto& node : nodes)
        {
            // Draw connections
            if (node->enabled)
            {
                const auto& myPos = (Vector2f(node->position) * gridSize) + (gridSize / 2);
                for (auto& n : node->neighbors)
                {
                    if (n->enabled == false) continue;
                    const auto& nPos = (Vector2f(n->position) * gridSize) + (gridSize / 2);

                    sf::Vertex line[] =
                    { sf::Vertex(myPos, sf::Color(255, 255, 255, 20)), sf::Vertex(nPos, sf::Color(255, 255, 255, 20)) };
                    target.draw(line, 2, sf::Lines);
                }
            }

            // Draw path
            if (!path.empty())
            {
                auto p1 = path.front();

                for (auto& point : path)
                {
                    if (point == p1) continue;
                    else
                    {
                        sf::Vertex line[] =
                        {
                            sf::Vertex(Vector2f(p1->position)    * gridSize + (gridSize / 2), sf::Color::Yellow),
                            sf::Vertex(Vector2f(point->position) * gridSize + (gridSize / 2), sf::Color::Yellow)
                        };
                        target.draw(line, 2, sf::Lines);

                        p1 = point;
                    }
                }
            }
        }

        // Draw the bodies last, so their on top
        for (auto& node : nodes)
        {
            // Node body
            nodeShape.setPosition(node->position.x * gridSize, node->position.y * gridSize);

            if (node->enabled == false)
            { nodeShape.setFillColor(sf::Color(25, 25, 25)); }
            else if (node.get() == start)
            { nodeShape.setFillColor(sf::Color::Green); }
            else if (node.get() == end)
            { nodeShape.setFillColor(sf::Color::Red); }
            else
            { nodeShape.setFillColor(sf::Color(std::clamp(node->moveCost * 10, 0.f, 255.f), 75, 75)); }

            target.draw(nodeShape);

            sf::Text costText(std::to_string(int(node->moveCost)), defaultFont, 10);
            costText.setPosition(node->position.x * gridSize, node->position.y * gridSize);
            target.draw(costText);
        }
    }

    void start()
    {
        if (!defaultFont.loadFromFile("Content/arial.ttf"))
        {
            std::cout << "Default font is missing!!";
            abort();
        }

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
                    auto neighborNode = pathfinder.getNodeByPosition(myPos + posDiff);
                    if (neighborNode != nullptr)
                    { node->connect(neighborNode); }
                }
            }
        }

        const float gridSize = 32; // Size of each grid in pixels
        Node* startPoint = pathfinder.getNodes().front().get();
        Node* endPoint   = pathfinder.getNodes().back().get();
        std::vector<Node*> pointPath;
        bool needToRecalcPath = true;

        sf::RenderWindow window(sf::VideoMode(1280, 720), "Test");
        while (window.isOpen())
        {
            // Input
            sf::Event e;
            while (window.pollEvent(e))
            {
                if (e.type == sf::Event::Closed) window.close();

                if (e.type == sf::Event::MouseButtonPressed || e.type == sf::Event::MouseWheelMoved)
                {
                    Vector2i clickedPos = Vector2i(sf::Mouse::getPosition(window)) / gridSize;
                    auto cp = pathfinder.getNodeByPosition(clickedPos);
                    if (cp == nullptr) { continue; }

                    if (e.type == sf::Event::MouseButtonPressed)
                    {
                        if (e.mouseButton.button == sf::Mouse::Left)
                        {
                            if   (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { cp->enabled = !cp->enabled; }
                            else { startPoint = cp; }
                            needToRecalcPath = true;
                        }
                        else if (e.mouseButton.button == sf::Mouse::Right)
                        {
                            endPoint = cp;
                            needToRecalcPath = true;
                        }
                    }

                    else if (e.type == sf::Event::MouseWheelMoved)
                    {
                        cp->moveCost = std::clamp(cp->moveCost + e.mouseWheel.delta, 1.f, 1000.f);
                        needToRecalcPath = true;
                    }
                }
            }

            // Update

            if (needToRecalcPath)
            {
                std::cout << "Path changed, recalculating... \n";
                needToRecalcPath = false;
                pointPath = pathfinder.findPath(startPoint, endPoint, true);

                if (pointPath.empty()) std::cout << "No path found!\n";
                //else for (auto& point : pointPath) { std::cout << point->position.toString() << std::endl; }

                std::cout << "Recalc complete. \n";
            }

            // Draw
            window.clear();

            drawNodes(window, pathfinder, startPoint, endPoint, pointPath, gridSize);

            window.display();
        }
    }
}

namespace StringHelpersTests
{
    void test_beginswith()
    {
        ASSERT(  strhelp::beginswith("a.png", "a") );
        ASSERT( !strhelp::beginswith("a.png", "b") );
        ASSERT(  strhelp::beginswith("aphoto.png", "aphoto.") );
        ASSERT(  strhelp::beginswith("jpeg.png", "jpeg") );
        ASSERT( !strhelp::beginswith("jpeg.png", "no") );

        printf("beginswith passed\n");
    }

    void test_endswith()
    {
        ASSERT(  strhelp::endswith("a.png", ".png") );
        ASSERT( !strhelp::endswith("a.png", "pg") );
        ASSERT(  strhelp::endswith("aphoto.png", "g") );
        ASSERT(  strhelp::endswith("jpeg.png", "png") );
        ASSERT( !strhelp::endswith("jpeg.png", "jpeg") );

        printf("endswith passed\n");
    }

    void test_trim()
    {
        std::string ex1 = "  trim  me pls   \t  ";
        ASSERT(strhelp::trimmed(ex1) == "trim  me pls");
        ASSERT(strhelp::trimmed(" LMAO") == "LMAO");
        ASSERT(strhelp::trimmed("getbodied    \n") == "getbodied");

        printf("trim passed\n");
    }

    void start()
    {
        test_beginswith();
        test_endswith();
        test_trim();
    }
}

int main()
{
    //AStarExample::start();
    //BreadthFirstSearchExample::start();
    StringHelpersTests::start();
}
