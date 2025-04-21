#ifndef TILE_HPP
#define TILE_HPP
#include "util.hpp"

class Tile
{
private:


public:
    int id;         // 磁盘页id

    int tag;        // 磁盘页标签

    int max_size;   // 页最大容量
    int cap_size;   // 页可用容量
    std::set<int> free_block;  // 页上的空闲块列表
    std::set<int> used_block;  // 页上的已用块列表


    std::vector<std::pair<int, int>> objList;  // 页上的对象列表,每个pair是对象id和对象blockid
    int start_pos;  // 页起始位置(闭区间)
    int end_pos;    // 页结束位置(开区间) = start_pos + max_size
    std::queue<std::pair<int, std::pair<int, int>>> request_map; // 该对象的请求列表，按request_id排序

    //构造函数
    Tile(int id, int start_pos, int max_size) : id(id), start_pos(start_pos), max_size(max_size) {
        cap_size = max_size;
        end_pos = start_pos + max_size;
        objList.resize(max_size);
        for(int i = 0; i < max_size; ++i) {
            free_block.insert(i);
        }
    }


    //在磁盘页上插入对象
    std::vector<int> insert_obj(int obj_id, int obj_size, std::vector<std::vector<std::pair<int, int>>> &tiles_list) {
        if(cap_size < obj_size) return {}; // 如果页可用容量不足，返回空
        std::vector<int> block_ids; // 存储对象的块id
        std::vector<std::pair<int, int>> tiles; // 存储对象的磁盘页id和偏移量

        for(int subobj_i = 0; subobj_i < obj_size; ++subobj_i) {
            int block_id = *free_block.begin(); // 获取空闲块id
            free_block.erase(block_id); // 从空闲块列表中删除
            used_block.insert(block_id); // 添加到已用块列表中
            objList[block_id] = {obj_id, subobj_i}; // 更新对象列表
            block_ids.emplace_back(block_id + start_pos); // 更新块id
            tiles.emplace_back(id, block_id); // 更新磁盘页id和偏移量
        }
        tiles_list.emplace_back(tiles); // 将对象存储的磁盘页id和偏移量存储到对象中
        cap_size -= obj_size; // 更新页可用容量

        return block_ids; // 返回块id
    }

    // 大的插小的
    std::vector<int> insert_obj_by_index_for_hot(int obj_id, int obj_size, std::vector<std::pair<int, int>>& tiles, std::vector<int> &block_ids, std::vector<int> &index) {
        // if(cap_size < obj_size) return {};
        std::vector<int> res;
        if(obj_size == block_ids.size()) {
            for(int subobj_i = 0; subobj_i < obj_size; ++subobj_i) {
                int block_id = block_ids[subobj_i] - start_pos; // 获取块id
                free_block.erase(block_id); // 从空闲块列表中删除
                used_block.insert(block_id); // 添加到已用块列表中
                objList[block_id] = {obj_id, subobj_i}; // 更新对象列表
                tiles[subobj_i].first = id; // 更新磁盘页id
                tiles[subobj_i].second = block_id; // 更新偏移量
                res.emplace_back(block_ids[subobj_i]); // 更新块id
                index[subobj_i] = subobj_i; // 更新索引
            }
        }else {
            int need_empty = obj_size - block_ids.size(); // 需要添加的空闲块数量
            int block_p = 0;
            auto it = free_block.begin();
            std::vector<int> insert_index; // 存储插入的索引
            while(need_empty && block_p < block_ids.size()) {
                if(*it < block_ids[block_p] - start_pos) {
                    insert_index.emplace_back(*it);
                    it ++;
                    need_empty -= 1;
                }else if(*it == block_ids[block_p] - start_pos){
                    insert_index.emplace_back(block_ids[block_p] - start_pos);
                    index[block_p] = insert_index.size() - 1; // 更新索引
                    block_p += 1;
                    it ++;
                }else {
                    insert_index.emplace_back(block_ids[block_p] - start_pos);
                    index[block_p] = insert_index.size() - 1; // 更新索引
                    block_p += 1;
                }
            }
            while(need_empty) {
                insert_index.emplace_back(*it);
                it ++;
                need_empty -= 1;
            }
            while(block_p < block_ids.size()) {
                insert_index.emplace_back(block_ids[block_p] - start_pos);
                index[block_p] = insert_index.size() - 1; // 更新索引
                block_p += 1;
            }
            ASSERT_DEBUG(insert_index.size() == obj_size, "插入对象失败！对象大小不齐\n", obj_id, obj_size, insert_index.size())
            for(int subobj_i = 0; subobj_i < obj_size; ++subobj_i) {
                free_block.erase(insert_index[subobj_i]); // 从空闲块列表中删除
                used_block.insert(insert_index[subobj_i]); // 添加到已用块列表中
                objList[insert_index[subobj_i]] = {obj_id, subobj_i}; // 更新对象列表
                tiles[subobj_i].first = id; // 更新磁盘页id
                tiles[subobj_i].second = insert_index[subobj_i]; // 更新偏移量
                res.emplace_back(insert_index[subobj_i] + start_pos); // 更新块id
            }
        }
        cap_size -= obj_size;
        ASSERT_DEBUG(res.size() == obj_size, "插入对象失败！对象大小不齐\n", obj_id, obj_size, res.size())
        return res;
    }

    // 小的插大的
    std::vector<int> insert_obj_by_index_for_aim(int obj_id, int obj_size, std::vector<std::pair<int, int>>& tiles, std::vector<int> &block_ids, std::vector<int> &index) {
        std::vector<int> res;
        for(int subobj_i = 0; subobj_i < obj_size; ++subobj_i) {
            int block_id = block_ids[index[subobj_i]] - start_pos; // 获取块id
            free_block.erase(block_id); // 从空闲块列表中删除
            used_block.insert(block_id); // 添加到已用块列表中
            objList[block_id] = {obj_id, subobj_i}; // 更新对象列表
            tiles[subobj_i].first = id; // 更新磁盘页id
            tiles[subobj_i].second = block_id; // 更新偏移量
            res.emplace_back(block_ids[index[subobj_i]]); // 更新块id
        }
        cap_size -= obj_size; // 更新页可用容量
        ASSERT_DEBUG(res.size() == obj_size, "插入对象失败！对象大小不齐\n", obj_id, obj_size, res.size())
        return res; // 返回块id
    }

    std::vector<int> insert_obj_by_free(int obj_id, int obj_size, std::vector<std::pair<int, int>>& tiles) {
        if(cap_size < obj_size) return {}; // 如果页可用容量不足，返回空
        std::vector<int> res;
        for(int subobj_i = 0; subobj_i < obj_size; ++subobj_i) {
            int block_id = *free_block.begin(); // 获取块id
            free_block.erase(block_id); // 从空闲块列表中删除
            used_block.insert(block_id); // 添加到已用块列表中
            objList[block_id] = {obj_id, subobj_i}; // 更新对象列表
            tiles[subobj_i].first = id; // 更新磁盘页id
            tiles[subobj_i].second = block_id; // 更新偏移量
            res.emplace_back(block_id + start_pos); // 更新块id
        }
        cap_size -= obj_size;
        ASSERT_DEBUG(res.size() == obj_size, "插入对象失败！对象大小不齐\n", obj_id, obj_size, res.size())
        return res;
    }

    //删除磁盘页上的对象
    bool delete_obj(int obj_id, int obj_size, std::vector<std::pair<int,int>> &block_ids) {
        // ASSERT_DEBUG(obj_size == block_ids.size(), "删除对象失败！对象大小不齐\n", obj_id, obj_size, block_ids.size())
        // ASSERT_DEBUG(id == block_ids[0].first, "删除对象失败！对象不在该磁盘页上\n", obj_id, id, block_ids[0].first)
        for(auto &[id, blockid] : block_ids) {
            free_block.insert(blockid); // 将块id添加到空闲块列表中
            used_block.erase(blockid); // 从已用块列表中删除
            objList[blockid] = {0, 0}; // 更新对象列表
        }
        cap_size += obj_size; // 更新页可用容量

        return true; // 删除成功
    }

    

    //删除磁盘页上的对象
    std::vector<int> delete_obj_by_index(int obj_id, int obj_size, std::vector<std::pair<int,int>> &block_ids) {
        ASSERT_DEBUG(obj_size == block_ids.size(), "删除对象失败！对象大小不齐\n", obj_id, obj_size, block_ids.size())
        ASSERT_DEBUG(id == block_ids[0].first, "删除对象失败！对象不在该磁盘页上\n", obj_id, id, block_ids[0].first)

        std::vector<int> res; // 存储删除的块id
        for(auto &[id, blockid] : block_ids) {
            res.emplace_back(blockid + start_pos); // 更新块id
            free_block.insert(blockid); // 将块id添加到空闲块列表中
            used_block.erase(blockid); // 从已用块列表中删除
            objList[blockid] = {0, 0}; // 更新对象列表
        }
        cap_size += obj_size; // 更新页可用容量
        ASSERT_DEBUG(res.size() == obj_size, "删除对象失败,删除的块id数量不齐\n", obj_id, obj_size, res.size())
        return res; // 删除成功
    }

    // 插入请求映射
    // void insert_requestmap(int request_id, int time, int object_size) {
    //     request_map[request_id] = {time, object_size};
    // }

    // 删除请求映射
    // void erase_requestmap(int request_id) {
    //     request_map.erase(request_id);
    // }


    
    // std::pair<int,int>& get_request_info (int request_id) {return request_map[request_id]; }

    // std::map<int, std::pair<int, int>>& get_requestmap() {return request_map; }
    inline bool check_use_status() {
        return free_block.size() == max_size; // 如果空闲块列表大小等于页最大容量，表示页是空闲的
    }

    inline std::unordered_set<int> get_obj_ids() {
        std::unordered_set<int> Object_ids; // 将已用块列表转换为vector
        for(auto &block_id : used_block) {
            Object_ids.insert(objList[block_id].first); // 获取对象id
        }
        return Object_ids;
    }

};

#endif // TILE_HPP