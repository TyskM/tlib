
# TLib
Some headers I use a lot.

## Changelog
- 3/31/24:
  - TLib/Renderer2D.hpp: Added drawTriangle function
  - TLib/Renderer2D.hpp: Added drawChar function
  - TLib/Renderer2D.hpp: Fixed texture bleeding
  - TLib/Containers/AStar2D: Added AStar2D.hpp
    - Included test "TLib/Tests/AStar2D.cpp" 
    - Only 8 directions for now
    - Prioritizes cardinal directions
    - Supports grid based raycasting (See: AStar2D::raycast())