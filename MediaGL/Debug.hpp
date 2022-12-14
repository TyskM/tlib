#pragma once

#include "../DataStructures.hpp"
#include "Renderer.hpp"
#include "Input.hpp"

void debugCamera(Renderer& r)
{
    static bool  dragging = false;
    static float minZoom  = 0.1f;
    static float maxZoom  = 5.f;
    static float zoomIncr = 0.15f;

    // Dragging
    if (Input::isMousePressed(Input::MOUSE_MIDDLE))
    {
        r._view.bounds.x -= Input::mouseDelta.x / r._view.zoom.x;
        r._view.bounds.y -= Input::mouseDelta.y / r._view.zoom.y;
    }

    // Zooming
    if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_DOWN))
    {
        r._view.zoom.x -= zoomIncr * r._view.zoom.x;
        r._view.zoom.y -= zoomIncr * r._view.zoom.y;
    }
    else if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_UP))
    {
        r._view.zoom.x += zoomIncr * r._view.zoom.x;
        r._view.zoom.y += zoomIncr * r._view.zoom.y;
    }
    r._view.zoom.x = std::clamp(r._view.zoom.x, minZoom, maxZoom);
    r._view.zoom.y = std::clamp(r._view.zoom.y, minZoom, maxZoom);

    r.setView(r._view);
}