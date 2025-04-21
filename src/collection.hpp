#ifndef COLLECTION_HPP
#define COLLECTION_HPP
#include "util.hpp"
#include "disk.hpp"
#include "object.hpp"
#include "heatmap.hpp"

class Garbage
{
private:
public:
    int object_id;
    int size;
    int tag;
    int cur_tile;
    int cur_tag;

    Garbage() {}
    
    Garbage(int object_id, int size, int tag, int cur_tile, int cur_tag)
        : object_id(object_id), size(size), tag(tag), cur_tile(cur_tile), cur_tag(cur_tag) {}

};


class Collection
{
private:
public:
    std::vector<std::unordered_map<int, Garbage>> garbage_storage;
    std::vector<std::unordered_map<int, std::unordered_set<int>>> tag_garbage_set;
    std::vector<std::unordered_map<int, std::unordered_set<int>>> tile_garbage_set;

    void output(std::vector<std::vector<std::pair<int, int>>>& res) {
        
        printf("GARBAGE COLLECTION\n");
        for (int i = 0; i < res.size(); ++i) {
            printf("%d\n", res[i].size()); // 输出每个磁盘的垃圾对象数
            for(int j = 0; j < res[i].size(); ++j) {
                printf("%d %d\n", res[i][j].first + 1, res[i][j].second + 1); // 磁盘id从1开始
            }
        }
    }

    void do_insert(int cur_time, int hot_obj_id, int aim_tile_id, Disk& disk, std::unordered_map<int, Object> &object_list, std::vector<int>& heatmap, std::vector<int> &num, std::vector<std::pair<int, int>>& res, std::unordered_map<int, Garbage>& sub_garbage_storage) {
        int hot_obj_tile = sub_garbage_storage[hot_obj_id].cur_tile; // 热垃圾对象所在磁盘页

        auto &hot_obj = object_list[hot_obj_id]; // 热垃圾对象

        // std::cerr << "do insert for " << hot_obj_id << " and tileid: " << aim_tile_id << std::endl;
        // std::cerr << "hot_obj_tile: " << hot_obj_tile << " aim_obj_tile: " << aim_tile_id << std::endl;

        int hot_obj_last_value = 0;
        int hot_obj_num = 0;
        auto &hot_tile = disk.tiles[hot_obj_tile]; // 热垃圾对象所在磁盘页
        auto hot_obj_req_map_it = hot_obj.request_map.begin();
        while (hot_obj_req_map_it != hot_obj.request_map.end()) {
            int value_lost = cur_time - hot_obj_req_map_it->second.timestamp + 1;
            int last_value = (105 - value_lost) > 0 ? (105 - value_lost) * hot_obj.size : 0;
            hot_obj_last_value += last_value; // 计算热垃圾对象的最后价值
            hot_obj_num += hot_obj.size;
            // hot_tile.request_map.push(std::make_pair(hot_obj_req_map_it->second.request_id, std::make_pair(hot_obj_req_map_it->second.timestamp, hot_obj.size))); // 将热垃圾对象的请求添加到目标磁盘页的请求列表中
            hot_obj_req_map_it++; // 移动到下一个请求
        }
        if(heatmap[hot_obj_tile] < hot_obj_last_value)
            heatmap[hot_obj_tile] = 0; // 更新热垃圾对象的热度图
        else
            heatmap[hot_obj_tile] -= hot_obj_last_value; // 更新热垃圾对象的热度图
        num[hot_obj_tile] -= hot_obj_num;


        heatmap[aim_tile_id] += hot_obj_last_value; // 更新热垃圾对象所在磁盘页的热度图
        num[aim_tile_id] += hot_obj_num;

        // std::cerr <<"update heatmap done!" << std::endl;

        int hot_garbage_tile_index = -1;
        for (int i = 0; i < REP_NUM; ++i) {
            if (hot_obj.disk[i] == disk.id) {
                hot_garbage_tile_index = i; // 获取热垃圾对象所在磁盘的索引
                // break;
            }
        }
        
        std::vector<int> old_block = disk.delete_obj_by_index(hot_obj_id, hot_obj.size, hot_obj.tiles[hot_garbage_tile_index], hot_obj_tile); // 删除热垃圾对象
        ASSERT_DEBUG(old_block.size() == hot_obj.size, "删除热垃圾对象失败！", hot_obj_id, hot_obj.size, old_block.size()) // 检查删除结果是否正确

        std::vector<int> new_block = disk.insert_obj_by_free(hot_obj_id, hot_obj.size, hot_obj.tiles[hot_garbage_tile_index], aim_tile_id); // 将热垃圾对象插入到目标磁盘页中
        // std::cerr << "insert done" <<std::endl;

        for(int i = 0; i < old_block.size(); ++i) {
            res.emplace_back(std::make_pair(old_block[i], new_block[i])); // 将删除和插入的结果存储到结果列表中
        }        
        
    }

    void do_exchange(int cur_time, int hot_obj_id, int aim_obj_id, Disk& disk, std::unordered_map<int, Object> &object_list, std::vector<int>& heatmap, std::vector<int> &num, std::vector<std::pair<int, int>>& res, std::unordered_map<int, Garbage>& sub_garbage_storage) {
        int hot_obj_tile = sub_garbage_storage[hot_obj_id].cur_tile; // 热垃圾对象所在磁盘页
        int aim_obj_tile = sub_garbage_storage[aim_obj_id].cur_tile; // 目标垃圾对象所在磁盘页  
        auto &hot_obj = object_list[hot_obj_id]; // 热垃圾对象
        auto &aim_obj = object_list[aim_obj_id]; // 目标垃圾对象

        // std::cerr << "do exchange for " << hot_obj_id << " and " << aim_obj_id << std::endl;
        // std::cerr << "hot_obj_tile: " << hot_obj_tile << " aim_obj_tile: " << aim_obj_tile << std::endl;

        
        int hot_obj_last_value = 0;
        int hot_obj_num = 0;
        auto hot_obj_req_map_it = hot_obj.request_map.begin();
        auto &hot_tile = disk.tiles[aim_obj_tile]; // 目标垃圾对象所在磁盘页
        while(hot_obj_req_map_it != hot_obj.request_map.end()) {
            int value_lost = cur_time - hot_obj_req_map_it->second.timestamp + 1;
            int last_value = (105 - value_lost) > 0 ? (105 - value_lost) * hot_obj.size : 0;
            hot_obj_last_value += last_value; // 计算热垃圾对象的最后价值
            hot_obj_num += hot_obj.size;
            // hot_tile.request_map.push(std::make_pair(hot_obj_req_map_it->second.request_id, std::make_pair(hot_obj_req_map_it->second.timestamp, hot_obj.size))); // 将热垃圾对象的请求添加到目标磁盘页的请求列表中
            hot_obj_req_map_it ++; // 移动到下一个请求
        }
        
        int aim_obj_last_value = 0;
        int aim_obj_num = 0;
        auto aim_obj_req_map_it = aim_obj.request_map.begin();
        auto &aim_tile = disk.tiles[hot_obj_tile]; // 热垃圾对象所在磁盘页
        while(aim_obj_req_map_it != aim_obj.request_map.end()) {
            int value_lost = cur_time - aim_obj_req_map_it->second.timestamp + 1;
            int last_value = (105 - value_lost) > 0 ? (105 - value_lost) * aim_obj.size : 0;
            aim_obj_last_value += last_value; // 计算目标垃圾对象的最后价值
            aim_obj_num += aim_obj.size;
            // aim_tile.request_map.push(std::make_pair(aim_obj_req_map_it->second.request_id, std::make_pair(aim_obj_req_map_it->second.timestamp, aim_obj.size))); // 将目标垃圾对象的请求添加到热垃圾对象所在磁盘页的请求列表中
            aim_obj_req_map_it ++; // 移动到下一个请求
        }
        
        if(heatmap[hot_obj_tile] > hot_obj_last_value) {
            heatmap[hot_obj_tile] -= hot_obj_last_value; // 更新热垃圾对象的热度图
        }else {
            heatmap[hot_obj_tile] = 0; // 如果热度图小于热垃圾对象的最后价值，则将热度图置为0
        }
        // heatmap[hot_obj_tile] -= hot_obj_last_value; // 更新热垃圾对象的热度图
        num[hot_obj_tile] -= hot_obj_num;

        // ASSERT_DEBUG(heatmap[hot_obj_tile] >= 0, "热垃圾对象的热度图小于0!", hot_obj_id, hot_obj_last_value, hot_obj_tile) // 检查热垃圾对象的热度图是否正确
        
        if(heatmap[aim_obj_tile] > aim_obj_last_value) {
            heatmap[aim_obj_tile] -= aim_obj_last_value; // 更新目标垃圾对象的热度图
        } else {
            heatmap[aim_obj_tile] = 0; // 如果热度图小于目标垃圾对象的最后价值，则将热度图置为0
        }
        // heatmap[aim_obj_tile] -= aim_obj_last_value; // 更新目标垃圾对象的热度图
        num[aim_obj_tile] -= aim_obj_num;

        // ASSERT_DEBUG(heatmap[aim_obj_tile] >= 0, "目标垃圾对象的热度图小于0!", aim_obj_id, aim_obj_last_value, aim_obj_tile) // 检查目标垃圾对象的热度图是否正确
        
        heatmap[hot_obj_tile] += aim_obj_last_value; // 更新热垃圾对象所在磁盘页的热度图
        num[hot_obj_tile] += aim_obj_num;

        heatmap[aim_obj_tile] += hot_obj_last_value; // 更新目标垃圾对象所在磁盘页的热度图
        num[aim_obj_tile] += hot_obj_num;

        int hot_garbage_tile_index = -1;
        int aim_garbage_tile_index = -1;
        for(int i = 0; i < REP_NUM; ++i) {
            if(hot_obj.disk[i] == disk.id) {
                hot_garbage_tile_index = i; // 获取热垃圾对象所在磁盘的索引
            }
            if(aim_obj.disk[i] == disk.id) {
                aim_garbage_tile_index = i; // 获取目标垃圾对象所在磁盘的索引
            }
        }

        std::vector<int> hot_old_block = disk.delete_obj_by_index(hot_obj_id, hot_obj.size, hot_obj.tiles[hot_garbage_tile_index], hot_obj_tile); // 删除热垃圾对象
        std::vector<int> aim_old_block = disk.delete_obj_by_index(aim_obj_id, aim_obj.size, aim_obj.tiles[aim_garbage_tile_index], aim_obj_tile); // 删除目标垃圾对象
        std::vector<int> index(aim_old_block.size(), -1); // 初始化索引
        std::vector<int> hot_new_block = disk.insert_obj_by_index_for_hot(hot_obj_id, hot_obj.size, hot_obj.tiles[hot_garbage_tile_index], aim_obj_tile, aim_old_block, index);
        std::vector<int> aim_new_block = disk.insert_obj_by_index_for_aim(aim_obj_id, aim_obj.size, aim_obj.tiles[aim_garbage_tile_index], hot_obj_tile, hot_old_block, index); // 将热垃圾对象插入到目标磁盘页中

        for(int i = 0; i < hot_new_block.size(); ++i) {
            res.emplace_back(std::make_pair(hot_old_block[i], hot_new_block[i])); // 将删除和插入的结果存储到结果列表中
            // std::cerr << "res_first: " << hot_old_block[i] << " res_second: " << hot_new_block[i] << std::endl;
        }


        // std::cerr << "exchange done" <<std::endl;
        
    }

    void handleGarbageRequest(std::vector<Disk> &disk_list, std::unordered_map<int, Object> &object_list, int cur_time, std::vector<std::vector<std::pair<int, int>>> &rank, int K, Heatmap &heatmap) {
        std::vector<int> tags_rank;
        for(int i = rank[0].size() - 1; i >= 0; --i) {
            tags_rank.emplace_back(rank[cur_time/FRE_PER_SLICING][i].first);
        }
        std::vector<std::vector<std::pair<int, int>>> output_res(disk_list.size());
        

        for(int garbage_disk_id = 0; garbage_disk_id < tag_garbage_set.size(); ++garbage_disk_id) { 
            // std::cerr << "now working on disk " << garbage_disk_id << std::endl;
            int switch_times = K;
            Disk &disk = disk_list[garbage_disk_id]; // 获取磁盘对象
            std::vector<std::pair<int, int>> &sub_output = output_res[garbage_disk_id]; // 获取磁盘对象的输出结果
            auto& sub_garbage_storage = garbage_storage[garbage_disk_id]; // 获取磁盘对象的垃圾存储

            for(const int &hot_tag : tags_rank) {
                // std::cerr << "now working on hot tag " << hot_tag << " switch_times left "<< switch_times << std::endl;
                if (tag_garbage_set[garbage_disk_id][hot_tag].empty()) continue; // 如果该标签没有垃圾数据，跳过

                auto hot_tag_garbage = tag_garbage_set[garbage_disk_id][hot_tag].begin(); // 获取该标签的垃圾数据集合
                while(hot_tag_garbage != tag_garbage_set[garbage_disk_id][hot_tag].end()) {
                    auto &hot_garbage_obj = sub_garbage_storage[*hot_tag_garbage]; // 热垃圾对象
                    // std::cerr << "now working on hot garbage obj " << hot_garbage_obj.object_id << std::endl;

                    if(hot_garbage_obj.size > switch_times) {
                        hot_tag_garbage ++; // 移动到下一个热垃圾对象
                        continue;
                    }

                    bool exchange_done = false; // 标记交换是否完成
                    bool insert_done = false; // 标记插入是否完成
                    

                    int start_tile_id = disk.tag_tiles[hot_tag].first; // 获取需要放回的磁盘页起始位置
                    int end_tile_id = disk.tag_tiles[hot_tag].second; // 获取需要放回的磁盘页结束位置
                    // std::cerr << "now search each tile garbage set" << std::endl;
                    // std::cerr << "search range from " << start_tile_id << " to " << end_tile_id << std::endl;
                    for(auto &[tile_id, tile_garbage] : tile_garbage_set[garbage_disk_id]) {
                        
                        if(tile_garbage.empty()) continue; // 如果该磁盘页没有垃圾数据，跳过
                        if(tile_id < start_tile_id || tile_id > end_tile_id) continue; // 如果磁盘页不在范围内，跳过
                        
                        // std::cerr << "now search aim obj at each tile" << std::endl;
                        auto aim_tile_garbage = tile_garbage.begin(); // 获取该磁盘页的垃圾数据集合
                        while(aim_tile_garbage != tile_garbage.end()) {
                            auto &aim_garbage_obj = sub_garbage_storage[*aim_tile_garbage]; // 查找目标垃圾对象
                            
                            if(hot_garbage_obj.cur_tag == aim_garbage_obj.tag && hot_garbage_obj.size <= aim_garbage_obj.size + disk.tiles[tile_id].cap_size && aim_garbage_obj.size <= hot_garbage_obj.size) {// 如果热垃圾对象可以放入目标磁盘页
                                /**
                                 * 实验性代码
                                 */
                                // if(hot_garbage_obj.cur_tag == aim_garbage_obj.tag) {
                                    do_exchange(cur_time, hot_garbage_obj.object_id, aim_garbage_obj.object_id, disk, object_list, heatmap.heatmap[garbage_disk_id], heatmap.num[garbage_disk_id], sub_output, sub_garbage_storage); // 交换对象
                                    switch_times -= hot_garbage_obj.size;
                                    hot_tag_garbage = erase(hot_tag_garbage, hot_garbage_obj.object_id, garbage_disk_id); // 删除热垃圾对象
                                    // ASSERT_DEBUG(hot_tag_garbage != tag_garbage_set[garbage_disk_id][hot_tag].end(), "删除热垃圾对象失败！", hot_garbage_obj.object_id, garbage_disk_id) // 检查删除结果是否正确
                                    // // std::cerr << "next hot_garbage_obj: " << *hot_tag_garbage << std::endl;
                                    erase_aim(aim_garbage_obj.object_id, garbage_disk_id); // 删除目标垃圾对象
                                    // std::cerr << "erase aim tag done!" << std::endl;
                                    exchange_done = true; // 标记交换完成
                                    break;
                                // }else{
                                //     do_exchange(cur_time, hot_garbage_obj.object_id, aim_garbage_obj.object_id, disk, object_list, heatmap.heatmap[garbage_disk_id], heatmap.num[garbage_disk_id], sub_output, sub_garbage_storage); // 交换对象
                                //     switch_times -= hot_garbage_obj.size;
                                //     int new_tag = hot_garbage_obj.cur_tag; // 获取新的标签
                                //     int new_tile = hot_garbage_obj.cur_tile; // 获取新的磁盘页
                                //     hot_tag_garbage = erase(hot_tag_garbage, hot_garbage_obj.object_id, garbage_disk_id); // 删除热垃圾对象
                                //     update_aim(aim_garbage_obj.object_id, garbage_disk_id, new_tag, new_tile); // 更新目标垃圾对象
                                //     exchange_done = true; // 标记交换完成
                                //     break;
                                // }
                            }else {
                                aim_tile_garbage ++; // 移动到下一个目标垃圾对象
                                continue;
                            }
                        }

                        if(exchange_done) break; // 如果交换完成，跳出循环                        
                    }

                    if(exchange_done) {
                        continue; // 如果交换完成，跳出循环
                    }

                    // 如果没有找到合适的磁盘页，需要存储在磁盘空闲区的tile中
                    // std::cerr << "exchange object not found ! now search empty tile" << std::endl;
                    // std::cerr << "search range from " << start_tile_id << " to " << end_tile_id << std::endl;
                    for(int i = start_tile_id; i <= end_tile_id; ++i) {
                        if(hot_garbage_obj.size <= disk.tiles[i].cap_size) {
                            do_insert(cur_time, hot_garbage_obj.object_id, i, disk, object_list, heatmap.heatmap[garbage_disk_id], heatmap.num[garbage_disk_id], sub_output, sub_garbage_storage); // 将热垃圾对象插入到目标磁盘页中
                            switch_times -= hot_garbage_obj.size; // 更新交换次数
                            hot_tag_garbage = erase(hot_tag_garbage, hot_garbage_obj.object_id, garbage_disk_id); // 删除热垃圾对象
                            insert_done = true; // 标记插入完成
                            break; // 跳出循环
                        }else {
                            continue;
                        }
                    }

                    if(insert_done) {
                        continue; // 如果插入完成，跳出循环
                    }

                    hot_tag_garbage ++; // 移动到下一个热垃圾对象

                }
                
            }
        }
        // std::cerr << "output res size: " << output_res.size() << std::endl;
        // for(int i = 0; i < output_res.size(); ++i) {
        //     std::cerr << "disk " << i << " output res size: " << output_res[i].size() << std::endl;
        //     for(int j = 0; j < output_res[i].size(); ++j) {
        //         std::cerr << "disk " << i << " output res: " << output_res[i][j].first << " " << output_res[i][j].second << std::endl;
        //     }
        // }

        output(output_res);
    }

    
    void insert(int disk_id, int tag, int object_id, int size, int tile_id, int cur_tag) {
        Garbage garbage(object_id, size, tag, tile_id, cur_tag); // 创建垃圾对象
        garbage_storage[disk_id].insert({object_id, garbage}); // 插入垃圾对象
        tag_garbage_set[disk_id][tag].insert(object_id); // 插入属于标签垃圾集合
        tile_garbage_set[disk_id][tile_id].insert(object_id); // 插入当前所在磁盘页垃圾集合
    }

    void init_garbage(int N) {
        garbage_storage.clear(); // 清空垃圾存储
        garbage_storage.resize(N); // 初始化垃圾存储
        tag_garbage_set.resize(N); // 初始化标签垃圾集合
        tile_garbage_set.resize(N); // 初始化磁盘页垃圾集合
    }

    std::unordered_set<int>::iterator erase(std::unordered_set<int>::iterator object_it, int object_id, int disk_id) {
        
        auto &object = garbage_storage[disk_id][object_id]; // 获取对象
        ASSERT_DEBUG(tag_garbage_set[disk_id][object.tag].count(object_id) > 0, "标签垃圾集合中没有该对象", object_id, disk_id) // 检查标签垃圾集合中是否有该对象
        ASSERT_DEBUG(tile_garbage_set[disk_id][object.cur_tile].count(object_id) > 0, "磁盘页垃圾集合中没有该对象", object_id, disk_id) // 检查磁盘页垃圾集合中是否有该对象
        ASSERT_DEBUG(*object_it == object_id, "迭代器指向的对象id与垃圾对象id不一致", *object_it, object_id) // 检查迭代器指向的对象id与垃圾对象id是否一致
        auto it = object_it;
        it++;
        tag_garbage_set[disk_id][object.tag].erase(object_it); // 删除标签垃圾集合
        tile_garbage_set[disk_id][object.cur_tile].erase(object_id); // 删除磁盘页垃圾集合
        garbage_storage[disk_id].erase(object_id); // 删除垃圾对象
        return it;
    }

    bool erase_aim(int object_id, int disk_id) {
        auto &object = garbage_storage[disk_id][object_id]; // 获取对象
        tag_garbage_set[disk_id][object.tag].erase(object_id); // 删除标签垃圾集合
        tile_garbage_set[disk_id][object.cur_tile].erase(object_id); // 删除磁盘页垃圾集合
        garbage_storage[disk_id].erase(object_id); // 删除垃圾对象  
        return true;      
    }

    bool update_aim(int object_id, int disk_id, int new_tag, int new_tile) {
        auto &object = garbage_storage[disk_id][object_id]; // 获取对象
        tile_garbage_set[disk_id][object.cur_tile].erase(object_id); // 删除磁盘页垃圾集合

        tile_garbage_set[disk_id][new_tile].insert(object_id); // 插入新的磁盘页垃圾集合
        object.cur_tag = new_tag; // 更新对象的标签
        object.cur_tile = new_tile; // 更新对象的磁盘页
        return true;

    }

    void deleteGarbage(int object_id, int disklen) {
        for(int disk_id = 0; disk_id < disklen; ++disk_id) {
            auto it = garbage_storage[disk_id].find(object_id); // 查找对象
            if (it != garbage_storage[disk_id].end()) {
                int tag = it->second.tag; // 获取标签
                int tile_id = it->second.cur_tile; // 获取磁盘页id
                ASSERT_DEBUG(tag_garbage_set[disk_id][tag].count(object_id) > 0, "标签垃圾集合中没有该对象", object_id, disk_id) // 检查标签垃圾集合中是否有该对象
                ASSERT_DEBUG(tile_garbage_set[disk_id][tile_id].count(object_id) > 0, "磁盘页垃圾集合中没有该对象", object_id, disk_id) // 检查磁盘页垃圾集合中是否有该对象
                tag_garbage_set[disk_id][tag].erase(object_id); // 删除标签垃圾集合
                tile_garbage_set[disk_id][tile_id].erase(object_id); // 删除磁盘页垃圾集合
                garbage_storage[disk_id].erase(it); // 删除垃圾对象
            }
        }
    }
    
};





#endif // COLLECTION_HPP