#pragma once

#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/DataStructures.hpp>
#include <TLib/Math.hpp>

void debugCamera(Camera2D& camera)
{
    static bool  dragging = false;
    static float minZoom  = 0.1f;
    static float maxZoom  = 5.f;
    static float zoomIncr = 0.15f;

    Rectf bounds = camera.getBounds();
    Vector2f zoom = camera.getZoom();

    // Dragging
    if (Input::isMousePressed(Input::MOUSE_MIDDLE))
    {
        bounds.x -= Input::mouseDelta.x / zoom.x;
        bounds.y -= Input::mouseDelta.y / zoom.y;
    }

    // Zooming
    if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_DOWN))
    {
        zoom.x -= zoomIncr * zoom.x;
        zoom.y -= zoomIncr * zoom.y;
    }
    else if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_UP))
    {
        zoom.x += zoomIncr * zoom.x;
        zoom.y += zoomIncr * zoom.y;
    }
    zoom.x = std::clamp(zoom.x, minZoom, maxZoom);
    zoom.y = std::clamp(zoom.y, minZoom, maxZoom);

    camera.setBounds(bounds);
    camera.setZoom(zoom);
}