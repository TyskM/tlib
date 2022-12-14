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

    auto view = r.getView();

    // Dragging
    if (Input::isMousePressed(Input::MOUSE_MIDDLE))
    {
        view.bounds.x -= Input::mouseDelta.x / view.zoom.x;
        view.bounds.y -= Input::mouseDelta.y / view.zoom.y;
    }

    // Zooming
    if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_DOWN))
    {
        view.zoom.x -= zoomIncr * view.zoom.x;
        view.zoom.y -= zoomIncr * view.zoom.y;
    }
    else if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_UP))
    {
        view.zoom.x += zoomIncr * view.zoom.x;
        view.zoom.y += zoomIncr * view.zoom.y;
    }
    view.zoom.x = std::clamp(view.zoom.x, minZoom, maxZoom);
    view.zoom.y = std::clamp(view.zoom.y, minZoom, maxZoom);

    r.setView(view);
}