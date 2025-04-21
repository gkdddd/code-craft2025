#ifndef DISK_HPP
#define DISK_HPP

#include "util.hpp"
#include "tile.hpp"
#include "point.hpp"

class Disk
{
private:


public:

    int id;         // 磁盘id
    int max_size;   // 磁盘最大容量
    int tile_size;  // 磁盘页大小
    int tile_nums;   // 磁盘页数
    std::unordered_map<int, std::pair<int, int>> tag_tiles;  // tag对应的磁盘页
    std::pair<int, int> default_val = {-1, -1}; // 默认值
    std::vector<Tile> tiles;  // 磁盘页列表
    Point points[POINT_NUM]; // 磁头列表

    //构造函数
    Disk(int id, int max_size) : id(id), max_size(max_size) {
        tile_size = TILE_SIZE; // 磁盘页大小
        tile_nums = max_size / tile_size; // 磁盘页数
        initTile(tile_size, tile_nums); // 初始化磁盘页
        initPoint(); // 初始化磁头
    }

    //初始化磁盘页
    bool initTile(int tile_size, int tile_nums) {
        this->tile_size = tile_size;
        this->tile_nums = tile_nums;
        for(int i = 0; i < tile_nums; ++i) {
            tiles.emplace_back(i, i * tile_size, tile_size);
        }
        return true;        
    }

    //初始化磁头
    bool initPoint() {
        for(int i = 0; i < POINT_NUM; ++i) {
            points[i] = Point(i, 0, 1, 1e5, G, 0);
        }
        return true;        
    }
    
    //在磁盘页上插入对象
    std::vector<int> insert_obj(int obj_id, int obj_size, std::vector<std::vector<std::pair<int, int>>> &obj, int tile_id) {
        return tiles[tile_id].insert_obj(obj_id, obj_size, obj); // 调用磁盘页的插入函数
    }

    //删除磁盘页上的对象
    bool delete_obj(int obj_id, int obj_size, std::vector<std::pair<int,int>> &obj, int tile_id) {
        return tiles[tile_id].delete_obj(obj_id, obj_size, obj); // 调用磁盘页的删除函数
    }

    std::vector<int> delete_obj_by_index(int obj_id, int obj_size, std::vector<std::pair<int,int>> &obj, int tile_id) {
        return tiles[tile_id].delete_obj_by_index(obj_id, obj_size, obj); // 调用磁盘页的删除函数
    }

    std::vector<int> insert_obj_by_index_for_hot(int obj_id, int obj_size, std::vector<std::pair<int, int>> &obj, int tile_id, std::vector<int> &blockids, std::vector<int> &index) {
        return tiles[tile_id].insert_obj_by_index_for_hot(obj_id, obj_size, obj, blockids, index);
    }
    
    std::vector<int> insert_obj_by_index_for_aim(int obj_id, int obj_size, std::vector<std::pair<int, int>> &obj, int tile_id, std::vector<int> &blockids, std::vector<int> &index) {
        return tiles[tile_id].insert_obj_by_index_for_aim(obj_id, obj_size, obj, blockids, index);
    }

    std::vector<int> insert_obj_by_free(int obj_id, int obj_size, std::vector<std::pair<int, int>> &obj, int tile_id) {
        return tiles[tile_id].insert_obj_by_free(obj_id, obj_size, obj); // 调用磁盘页的插入函数
    }

    
    //获取指定tag磁盘页的范围
    std::pair<int, int>& get_tile_range(int tag) {
        auto it = tag_tiles.find(tag);
        if (it != tag_tiles.end()) {
            return it->second; // 返回磁盘页范围
        } else {
            return default_val; // 返回默认值
        }
    }

    void set_tile_range(int tag, int start, int end) {
        tag_tiles[tag] = {start, end}; // 设置磁盘页范围
        for(int i = start; i <= end; ++i) {
            tiles[i].tag = tag; // 设置磁盘页标签
        }
    }
    
    int& get_id() { return id; } // 获取磁盘id
    std::pair<int, int>& get_default_range() { return default_val; } // 获取磁盘页范围
    int& get_tile_nums() { return tile_nums; } // 获取磁盘页数
    int& get_max_size() { return max_size; } // 获取磁盘最大容量
    Tile& getTile(int tile_id) {return tiles[tile_id]; } // 获取磁盘页对象
    Point& getPoint(int point_id) {return points[point_id]; } // 获取磁头对象

    int& getTileSize() {return tile_size;}
};




#endif // DISK_HPP