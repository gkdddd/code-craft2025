#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "util.hpp"
#include "request.hpp"

class Object
{
private:



public:

    int object_id;
    int size;
    int tag; //对象的标签
    //记录所有副本
    int disk[REP_NUM]; // disk[i]表示第i个副本所在的磁盘
    std::vector<std::vector<std::pair<int, int>>> tiles; //每个磁盘下存储的tile id以及具体的位置

    std::unordered_map<int, int> block_to_timestamp; // block_id到时间戳的映射
    std::map<int, Request> request_map; // 该对象的请求列表，按request_id排序

    
    Object() {}
    Object(int object_id, int size, int tag) : object_id(object_id), size(size), tag(tag) {
        for(int i = 0; i < REP_NUM; ++i) {
            disk[i] = -1;
        }
        tiles.resize(REP_NUM);
        for (auto& tile : tiles) {
            tile.resize(size);
        }
    }


    // 打包对象
    void packObject(std::vector<std::vector<std::pair<int, int>>>&& tilesinfo, std::vector<std::pair<int, std::vector<int>>>& obj_point) { // 打包对象
        tiles = std::move(tilesinfo);
        for(int i = 0; i < REP_NUM; ++i) {
            disk[i] = obj_point[i].first;
        }
    }

    std::map<int, Request> &getRequestMap() { return request_map; } // 获取请求列表
    int& getDiskId(int i) { return disk[i]; } // 获取磁盘id
    int& getObjectId() { return object_id; } // 获取对象id
    int& getSize() { return size; } // 获取对象大小
    int& getTag() { return tag; } // 获取对象标签
    std::vector<std::pair<int, int>> &getTiles(int i) { return tiles[i]; } // 获取对象在磁盘上的位置



};







#endif // OBJECT_HPP