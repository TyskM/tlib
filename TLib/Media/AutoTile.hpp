
// TODO: test and refactor this trash.

#pragma once

#pragma once
#include <TLib/Types/Types.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/Containers/GridMap2D.hpp>
#include <TLib/Media/Resource/Texture.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

template <typename K, typename V>
void to_json(json& j, const UnorderedMap<K, V>& map)
{
    for(auto& [k, v] : map)
    { j.push_back({std::to_string(k), v}); }
}

// If key is integral
template <typename K, typename V>
void from_json(const json& j, UnorderedMap<K, V>& map)
{
    for (auto& arr : j)
    { map[atoi(arr[0].get<String>().c_str())] = arr[1].get<V>(); }
}

template <typename T>
void to_json(json& j, const Vector2<T>& v)
{
    j = json{v.x, v.y};
}

template <typename T>
void from_json(const json& j, Vector2<T>& v)
{
    j.at(0).get_to(v.x);
    j.at(1).get_to(v.y);
}

using TilesetID        = int32_t;
TilesetID NullTileset  = -1;

struct Tile
{
     int32_t id        = NullTileset; // Index into a container with definitions
    uint32_t variation = 0; // TODO: Use variation for random tiles
    uint8_t  mask      = 0; // Mask for tracking neighbors
};

class Tileset
{
public:
    UnorderedMap<uint8_t, Vector2i> lut;
    Vector2i                        cellSize;
    Vector2i                        atlasSize;

    enum class Code : uint8_t
    {
        Success,
        DataFileDoesNotExist,
        InvalidDataInternals
    };

    Vector2i at(uint8_t bitmask) const
    {
        Vector2i cellPos;
        if      (lut.contains(bitmask)) { return lut.at(bitmask); }        // Try to return the exact tile
        else if (lut.contains(0))       { return lut.at(0); }             // If we can't then return a "neutral" tile
        else if (lut.size() > 0)        { return lut.begin()->second; }  // If not, then try to return any tile at all
        return Vector2i();                                              // We tried our best
    }

    Code loadFromFile(Path dataPath)
    {
        // why in gods name does replace_extension mutate the object zzzzz
        // and then it returns itself, who the fuck wrote this???
        if (dataPath.extension() != ".json")
            dataPath = dataPath.replace_extension(".json");

        if (!fs::exists(dataPath)) { return Code::DataFileDoesNotExist; }

        // Parse Json
        json tileData;
        {
            try
            { tileData = json::parse(readFile(dataPath), nullptr, true, true); }
            catch (const json::exception& e)
            { tlog::error("Error parsing json: {}", e.what()); return Code::InvalidDataInternals; }
        }

        from_json(tileData);

        return Code::Success;
    }

    void to_json(json& js) const
    {
        js = { { "atlasSize", atlasSize }, { "cellSize", cellSize }, { "lut", lut } };
    }

    void from_json(const json& js)
    {
        js.at("atlasSize").get_to(atlasSize);
        js.at("cellSize") .get_to(cellSize);
        js.at("lut")      .get_to(lut);
    }
};

class Tilemap
{
    mutable GridMap2D<Tile> tiles;
    mutable bool            dirty = true;

    uint8_t neighborBits(
    bool t,
    bool b,
    bool l,
    bool r,
    bool tl,
    bool tr,
    bool bl,
    bool br) const
    {
        // https://www.youtube.com/watch?v=mQRokJfkLY4
        if (!(t && l)) { tl = false; }
        if (!(t && r)) { tr = false; }
        if (!(b && l)) { bl = false; }
        if (!(b && r)) { br = false; }

        uint8_t total = 0;
        if (t)  { total |= bit(0); }
        if (tr) { total |= bit(1); }
        if (r)  { total |= bit(2); }
        if (br) { total |= bit(3); }
        if (b)  { total |= bit(4); }
        if (bl) { total |= bit(5); }
        if (l)  { total |= bit(6); }
        if (tl) { total |= bit(7); }
        return total;
    }

    // All the magic happens here
    void update() const
    {
        for (uint32_t x = 0; x < tiles.width();  x++) {
        for (uint32_t y = 0; y < tiles.height(); y++)
        {
            Vector2i pos(x, y);
            auto& tile = tiles.at(pos);
            if (tile.id == NullTileset) { continue; }

            const Vector2i tp  = pos + Vector2i( 0,  1); 
            const Vector2i trp = pos + Vector2i( 1,  1);
            const Vector2i rp  = pos + Vector2i( 1,  0);
            const Vector2i brp = pos + Vector2i( 1, -1);
            const Vector2i bp  = pos + Vector2i( 0, -1);
            const Vector2i blp = pos + Vector2i(-1, -1);
            const Vector2i lp  = pos + Vector2i(-1,  0);
            const Vector2i tlp = pos + Vector2i(-1,  1);

            bool t  = tiles.inBounds(tp ) && tiles.at(tp ).id != NullTileset;
            bool tr = tiles.inBounds(trp) && tiles.at(trp).id != NullTileset;
            bool r  = tiles.inBounds(rp ) && tiles.at(rp ).id != NullTileset;
            bool br = tiles.inBounds(brp) && tiles.at(brp).id != NullTileset;
            bool b  = tiles.inBounds(bp ) && tiles.at(bp ).id != NullTileset;
            bool bl = tiles.inBounds(blp) && tiles.at(blp).id != NullTileset;
            bool l  = tiles.inBounds(lp ) && tiles.at(lp ).id != NullTileset;
            bool tl = tiles.inBounds(tlp) && tiles.at(tlp).id != NullTileset;

            tile.mask = neighborBits(t, b, l, r, tl, tr, bl, br);
        }}
    }

public:
    Tilemap(uint32_t x, uint32_t y) { resize(x, y); }
    Tilemap() = default;

    void resize(uint32_t x, uint32_t y) { tiles.resize(x, y); }

    bool inBounds(const Vector2i& pos) const { return tiles.inBounds(pos); }

    Vector2i size()   const { return tiles.size();   }
    auto     width()  const { return tiles.width();  }
    auto     height() const { return tiles.height(); }

    // Use NullTileset to unset
    void set(uint32_t x, uint32_t y, TilesetID tilesetID)
    {
        if (!inBounds(Vector2i(x, y))) { return; }
        auto& tile = tiles.at(x, y);
        if (tile.id == tilesetID) { return; }
        tile.id = tilesetID;
      //tile.variation;
        dirty = true;
    }

    void fill(TilesetID tilesetID)
    {
        auto size = tiles.size();
        for (size_t x = 0; x < size.x; x++) {
        for (size_t y = 0; y < size.y; y++)
        { set(x, y, tilesetID); }}
    }

    void fillRect(Vector2i a, Vector2i b, TilesetID tilesetID)
    {
        a.x = std::clamp(a.x, 0, tiles.width());
        a.y = std::clamp(a.y, 0, tiles.height());
        b.x = std::clamp(b.x, 0, tiles.width());
        b.y = std::clamp(b.y, 0, tiles.height());

        if (a.x > b.x) { std::swap(a.x, b.x); }
        if (a.y > b.y) { std::swap(a.y, b.y); }

        for (uint32_t x = a.x; x < b.x; x++) {
        for (uint32_t y = a.y; y < b.y; y++)
        {
            set(x, y, tilesetID);
        }}
    }

    uint8_t getMask(uint32_t x, uint32_t y, const Tileset& tileset) const
    {
        if (dirty) { update(); dirty = false; }
        auto& tile = tiles.at(x, y);
        if (tile.id != NullTileset)
        { return tile.mask; }
        return 0;
    }

    Vector2i getGridCoords(uint32_t x, uint32_t y, const Tileset& tileset) const
    {
        uint8_t mask = getMask(x, y, tileset);
        return tileset.at(mask);
    }

    Vector2f getPixelCoords(uint32_t x, uint32_t y, const Tileset& tileset) const
    {
        Vector2i gridCoords = getGridCoords(x, y, tileset);
        return Vector2f(gridCoords.x, gridCoords.y) * Vector2f(tileset.cellSize);
    }

    Rectf getPixelRect(uint32_t x, uint32_t y, const Tileset& tileset) const
    {
        Vector2f pxCoords = getPixelCoords(x, y, tileset);
        return Rectf(pxCoords, Vector2f(tileset.cellSize));
    }

    // For debugging
    Tile& getTileForPos(uint32_t x, uint32_t y) const
    {
        if (dirty) { update(); dirty = false; }
        return tiles.at(x, y);
    }
};
