#pragma once

#include <Corrade/Containers/Optional.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Shaders/Generic.h>
#include <Magnum/Shaders/Shaders.h>
#include <Magnum/Math/Color.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Renderer.h>
#include "../Macros.hpp"
#include "../DataStructures.hpp"

Magnum::Platform::Application::Arguments getEmptyArgs()
{
    static int zero = NULL;
    Magnum::Platform::Application::Arguments args(zero, NULL);
    return args;
}

Magnum::PluginManager::Manager<Magnum::Trade::AbstractImporter> manager;