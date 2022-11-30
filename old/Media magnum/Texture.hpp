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
#include <Magnum/Math/Matrix3.h>
#include <Magnum/Math/Matrix4.h>
#include <MagnumPlugins/StbImageImporter/StbImageImporter.h>
#include <MagnumPlugins/StbImageConverter/StbImageConverter.h>
#include "../Macros.hpp"
#include "../DataStructures.hpp"

struct Texture
{
    static inline Magnum::Trade::StbImageImporter anyImageImporter;

    using Filtering = Magnum::GL::SamplerFilter;
    using Wrapping  = Magnum::GL::SamplerWrapping;

    void loadFromFile(std::string path, Filtering filtering = Filtering::Nearest, Wrapping wrapping = Wrapping::ClampToEdge)
    {
        if (!anyImageImporter.openFile(path))
        { std::cerr << "Couldn't open file: " << path << std::endl; abort(); }

        Magnum::Containers::Optional<Magnum::Trade::ImageData2D> image = anyImageImporter.image2D(0);
        CORRADE_INTERNAL_ASSERT(image);
        texture.setWrapping(wrapping);
        texture.setMagnificationFilter(filtering);
        texture.setMinificationFilter(filtering);
        texture.setStorage(1, Magnum::GL::textureFormat(image->format()), image->size());
        texture.setSubImage(0, {}, *image);
    }

    Magnum::GL::Texture2D texture;

    operator Magnum::GL::Texture2D&() { return  texture; }
    operator Magnum::GL::Texture2D*() { return &texture; }
};
