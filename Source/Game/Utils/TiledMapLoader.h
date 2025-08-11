#pragma once
#include "json.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <filesystem>

using json = nlohmann::json;

//カスタムプロパティ
struct Property {
    std::string name;
    std::string type;
    std::variant<int, float, bool, std::string> value;
    void GetValue(int& data) const { data = std::get<int>(value); }
    void GetValue(float& data) const { data = std::get<float>(value); }
    void GetValue(bool& data) const { data = std::get<bool>(value); }
    void GetValue(std::string& data) const { data = std::get<std::string>(value); }
};

//タイル情報
struct Tile {
    int id;
    //std::string name;
    std::map<std::string, Property> properties;
};

//タイルセット（同じ種類のオブジェクト）
struct Tileset {
    int firstGid = 1;
    int tileCount;
    std::string name;
    std::vector<Tile> tiles;
};

//タイルレイヤーデータ
struct TileLayer {
    std::string name;
    int width = 0;
    int height = 0;
    std::vector<int> tiles;
};

//マップ全体（全ステージデータ）
struct TileMap {
    int width = 0;
    int height = 0;
    int tileWidth = 0;
    int tileHeight = 0;
    std::map<std::string, std::vector<TileLayer>> groups;
    std::map<std::string, TileLayer> layers;
    std::vector<Tileset> tilesets;
};

class TiledMapLoader 
{
public:
    static inline std::string dataDir;

    static TileMap LoadFromFile(const std::string& filename, const std::string& dir = {}) {
        dataDir = dir;
        json json = LoadJson(dir + filename);
        return LoadTileMap(json);
    }

    static std::vector<std::string> LoadStageNames(const std::string& filePath) {
        std::vector<std::string> names;
        json json = LoadJson(filePath);
        for (const auto& group : json["layers"]) {
            if (group["type"] == "group") {
                names.push_back(group["name"]);
            }
        }
        return names;
    }

private:

    static json LoadJson(const std::string& filePath) {
        std::ifstream file(filePath);
        _ASSERT_EXPR(std::filesystem::exists(std::filesystem::path(filePath)), "ファイルを開けませんでした: " + filePath);

        json j;
        file >> j;
        return j;
    }

    static TileMap LoadTileMap(const json& j) {
        TileMap map;
        map.width = j["width"];
        map.height = j["height"];
        map.tileWidth = j["tilewidth"];
        map.tileHeight = j["tileheight"];

        //group,tilelayer読み込み
#if 0
        for (const auto& group : j["layers"]) {
            if (group["type"] == "group") {
                TileGroup tg;
                tg.name = group["name"];
                tg.layers = LoadTileLayers(group);
                map.groups.push_back(tg);
            }
        }
#else
        for (const auto& group : j["layers"]) {
            if (group["type"] == "group") {
                map.groups[group["name"]] = LoadTileLayers(group);
            }
            else if (group["type"] == "tilelayer") {
                for (TileLayer& tl : LoadTileLayers(j)) {
                    map.layers[tl.name] = tl;
                }
                break;
            }
        }
#endif // 0

        //tilesets読み込み
        map.tilesets = LoadTilesets(j);
        //map.tiles = LoadTilesetsData(j);

        return map;
    }

    static std::vector<TileLayer> LoadTileLayers(const json& j) {
        std::vector<TileLayer> layers;
        for (const auto& layer : j["layers"]) {
            std::string type = layer["type"];
            if (type == "tilelayer") {
                TileLayer tl;
                tl.name = layer["name"];
                tl.width = layer["width"];
                tl.height = layer["height"];
                std::vector<int> tiles = (layer["data"]);
                tl.tiles = tiles;
                layers.push_back(tl);
            }
        }
        return layers;
    }


    static std::vector<Tileset> LoadTilesets(const json& j) {
        std::vector<Tileset> tilesets;
        for (const auto& tileset : j["tilesets"]) {
            std::string sourcePath = (std::filesystem::path(dataDir) / tileset["source"].get<std::string>()).string();
            json source = LoadJson(sourcePath);
            if (!source.is_null()) {
                if (source["type"] == "tileset") {
                    Tileset ts;
                    ts.firstGid = tileset["firstgid"];
                    ts.tileCount = source["tilecount"];
                    ts.name = source["name"];
                    ts.tiles = LoadTiles(source, tileset["firstgid"]);
                    tilesets.push_back(ts);
                }
            }
        }
        return tilesets;
    }


    static std::vector<Tile> LoadTiles(const json& j, int firstGid) {
        std::vector<Tile> tiles;
        if (j.contains("tiles")) { // キーが存在するか確認
            for (const auto& data : j["tiles"]) {
                int id = firstGid + data["id"];
                Tile& t = tiles.emplace_back();
                t.id = id;
                //t.name = data["name"];
                if (data.contains("properties")) {
                    for (const auto& property : data["properties"]) {
                        Property& p{ t.properties[property["name"]]};
                        p.name = property["name"];
                        p.type = property["type"];
                        const auto& value = property["value"];

                        if (p.type == "string") {
                            p.value = value.get<std::string>();
                        }
                        else if (p.type == "int") {
                            p.value = value.get<int>();
                        }
                        else if (p.type == "bool") {
                            p.value = value.get<bool>();
                        }
                        else if (p.type == "float") {
                            p.value = value.get<float>();
                        }
                        else {
                            _ASSERT_EXPR(false, "未対応のプロパティタイプ: " + p.type);
                        }
                    }
                }
            }
        }
        return tiles;
    }
};