#pragma once

#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Media/Resource/Asset.hpp>
#include <TLib/Containers/UnorderedSet.hpp>
#include <TLib/Containers/Deque.hpp>
#include <TLib/Containers/Hive.hpp>
#include <TLib/Files.hpp>
#include <TLib/Logging.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/thirdparty/RectPack2D.hpp>
#include <TLib/Media/Renderer.hpp>

using namespace rectpack2D;

struct RectPackerOnline
{
    // https://web.archive.org/web/20150313230209/http://clb.demon.fi/projects/rectangle-bin-packing

    using Dim_t = int32_t;

    struct Node : NonAssignable
    {
        // Left and right child. We don't really distinguish which is which, so these could
        // as well be child1 and child2.
        Node* left  = nullptr;
        Node* right = nullptr;

        Path path;

        Dim_t x      = 0;
        Dim_t y      = 0;
        Dim_t width  = 0;
        Dim_t height = 0;
    };

    void init(Dim_t width, Dim_t height)
    {
        _nodes.emplace();
        binWidth      = width;
        binHeight     = height;
        root().left   = root().right = 0;
        root().x      = root().y = 0;
        root().width  = width;
        root().height = height;
    }

    Node* insert(Dim_t width, Dim_t height)
    {
        return insert(&root(), width, height);
    }

    /** @return A value [0, 1] denoting the ratio of total surface area that is in use.
    0.0f - the bin is totally empty, 1.0f - the bin is full. */
    float occupancy() const
    {
        unsigned long totalArea = binWidth * binHeight;
        unsigned long usedArea = usedSurfaceArea(root());
        return (float)usedArea/totalArea;
    }

    const auto& nodes() const
    { return _nodes; }

private:
    Hive<Node> _nodes;

    const Node& root() const { return *_nodes.begin(); }
    Node& root() { return *_nodes.begin(); }

// The total size of the bin we started with.
    Dim_t binWidth;
    Dim_t binHeight;

    /// @return The surface area used by the subtree rooted at node.
    unsigned long usedSurfaceArea(const Node& node) const
    {
        if (node.left || node.right)
        {
            unsigned long usedArea = node.width * node.height;
            if (node.left)  usedArea += usedSurfaceArea(*node.left);
            if (node.right) usedArea += usedSurfaceArea(*node.right);

            return usedArea;
        }

        // This is a leaf node, it doesn't constitute to the total surface area.
        return 0;
    }

    /// Inserts a new rectangle in the subtree rooted at the given node.
    Node* insert(Node* node, Dim_t width, Dim_t height)
    {
        // If this node is an internal node, try both leaves for possible space.
        // (The rectangle in an internal node stores used space, the leaves store free space)
        if (node->left || node->right)
        {
            if (node->left)
            {
                Node* newNode = insert(node->left, width, height);
                if (newNode)
                    return newNode;
            }
            if (node->right)
            {
                Node* newNode = insert(node->right, width, height);
                if (newNode)
                    return newNode;
            }
            return 0; // Didn't fit into either subtree!
        }

        // This node is a leaf, but can we fit the new rectangle here?
        if (width > node->width || height > node->height)
            return 0; // Too bad, no space.

        // The new cell will fit, split the remaining space along the shorter axis,
        // that is probably more optimal.
        Dim_t w = node->width - width;
        Dim_t h = node->height - height;

        node->left  = &*_nodes.emplace();
        node->right = &*_nodes.emplace();

        if (w <= h) // Split the remaining space in horizontal direction.
        {
            node->left->x       = node->x + width;
            node->left->y       = node->y;
            node->left->width   = w;
            node->left->height  = height;

            node->right->x      = node->x;
            node->right->y      = node->y + height;
            node->right->width  = node->width;
            node->right->height = h;
        }
        else // Split the remaining space in vertical direction.
        {
            node->left->x       = node->x;
            node->left->y       = node->y + height;
            node->left->width   = width;
            node->left->height  = h;

            node->right->x      = node->x + width;
            node->right->y      = node->y;
            node->right->width  = w;
            node->right->height = node->height;
        }
        // Note that as a result of the above, it can happen that node->left or node->right
        // is now a degenerate (zero area) rectangle. No need to do anything about it,
        // like remove the nodes as "unnecessary" since they need to exist as children of
        // this node (this node can't be a leaf anymore).

        // This node is now a non-leaf, so shrink its area - it now denotes
        // *occupied* space instead of free space. Its children spawn the resulting
        // area of free space.
        node->width = width;
        node->height = height;
        return node;
    }
};

// Supply and stitch textures one at a time.
struct TextureStitcherOnline
{
    RectPackerOnline packer;
    Texture          atlas;

    SubTexture load(const Path& path)
    {
        // TODO: Resize/New texture when atlas is full
        if (!atlas.created()) { init(); }

        auto existingNode =
            eastl::find_if(packer.nodes().begin(), packer.nodes().end(),
                [&path](const RectPackerOnline::Node& v) { return (!v.path.empty()) && fs::equivalent(v.path, path); });

        ASSERT(!existingNode->path.empty());
        if (existingNode != packer.nodes().end())
        {
            return SubTexture(atlas, Rectf(Vector2f(existingNode->x, existingNode->y), Vector2f(existingNode->width, existingNode->height)));
        }

        TextureData data;
        data.loadFromPath(path);
        auto size  = data.size();
        auto node  = packer.insert(size.x, size.y);
        node->path = path;
        atlas.setSubData(data, node->x, node->y);
        return SubTexture(atlas, Rectf(Vector2f(node->x, node->y), Vector2f(size)));
    }

    auto& getAtlas()
    { return atlas; }

    void init(int32_t initialSizeX = 1024, int32_t initialSizeY = 1024)
    {
        packer.init(initialSizeX, initialSizeY);
        atlas.create();
        atlas.setData(NULL, initialSizeX, initialSizeY, TexPixelFormats::RGBA, TexInternalFormats::RGBA);
        atlas.setFilter(TextureMinFilter::Nearest, TextureMagFilter::Nearest);
        atlas.setUVMode(UVMode::Repeat);
    }

};

// Supply all textures up front, then stitch
class TextureStitcherOffline
{
    bool stitched = false;
    Vector<Path> texturesToLoad;
    Deque<SubTexture> sprites;
    Texture atlas;

    using spaces_type = rectpack2D::empty_spaces<false>;
    using rect_type = output_rect_t<spaces_type>;
    std::vector<rect_type> rects;

public:
    // For debug
    Texture& getAtlas() { return atlas; }

    Asset<SubTexture> addTexture(const Path& path)
    {
        for (size_t i = 0; i < texturesToLoad.size(); i++)
        {
            if (fs::equivalent(texturesToLoad[i], path))
            { return Asset<SubTexture>(sprites[i]); }
        }

        texturesToLoad.push_back(path);
        sprites.emplace_back();
        auto& newSprite = sprites.back();
        newSprite.texture = &atlas;
        return Asset<SubTexture>(newSprite);
    }

    bool stitch()
    {
        tlog::info("Stitching textures...");
        ASSERT(!stitched);
        ASSERT(sprites.size() == texturesToLoad.size());

        const auto maxTexSize    = Renderer::getMaxTextureSize();
        const auto discard_step  = -4;
        auto report_successful   = [](rect_type&) { return callback_result::CONTINUE_PACKING; };
        auto report_unsuccessful = [](rect_type&) { return callback_result::ABORT_PACKING; };
        rects.clear();
        rects.resize(texturesToLoad.size());
        Vector<TextureData> textures;
        textures.resize(texturesToLoad.size());

        for (uint32_t i = 0; i < textures.size(); i++)
        {
            auto& path = texturesToLoad[i];
            auto& tex  = textures[i];
            tlog::info("Loading texture '{}'", path.string());
            tex.loadFromPath(path);
            auto  texSize = tex.size();
            auto& rect = rects[i];
            rect.w = texSize.x;
            rect.h = texSize.y;
        }

        const auto resultSize = find_best_packing<spaces_type>(rects,
            make_finder_input(maxTexSize, discard_step, report_successful, report_unsuccessful, flipping_option::DISABLED));
        if (resultSize.w > maxTexSize || resultSize.h > maxTexSize)
        {
            tlog::critical("Too many/big textures for texture stitcher.");
            return false;
        }

        atlas.create();
        atlas.setData(NULL, resultSize.w, resultSize.h, TexPixelFormats::RGBA, TexInternalFormats::RGBA);
        atlas.setFilter(TextureMinFilter::NearestMipmapNearest, TextureMagFilter::Nearest);
        atlas.setUVMode(UVMode::Repeat);

        for (uint32_t i = 0; i < textures.size(); i++)
        {
            auto& tex  = textures[i];
            auto& rect = rects[i];
            auto& path = texturesToLoad[i];
            tlog::info("Stitching texture '{}'", path.string());
            atlas.setSubData(tex, rect.x, rect.y);
            auto& sprite  = sprites[i];
            sprite.rect = Rectf(rect.x, rect.y, rect.w, rect.h);
        }

        atlas.generateMipmaps();

        stitched = true;
        return true;
    }
};