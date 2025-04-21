#ifndef DELETE_HPP
#define DELETE_HPP

#include "util.hpp"
#include "disk.hpp"
#include "object.hpp"
#include "request.hpp"
#include "heatmap.hpp"
#include "collection.hpp"   

class DeleteList
{
private:
public:
    // 从disk中彻底删除对象及其副本
    bool popObject(Object &object, std::vector<Disk> &disk_list) {
        for(int i = 0; i < REP_NUM; ++i) {
            int disk_id = object.getDiskId(i); // 获取磁盘id
            Disk &disk = disk_list[disk_id]; // 获取磁盘对象
            int object_id = object.getObjectId(); // 获取对象id
            int object_size = object.getSize(); // 获取对象大小
            auto& object_tile = object.getTiles(i); // 获取对象在磁盘上的位置
            if(!disk.delete_obj(object_id, object_size, object_tile, object_tile[0].first)) { // 删除对象
                return false; // 删除失败
            }
        }
        return true; // 删除成功
    }

    void output(std::vector<int> &abort_list, std::unordered_set<int>& done_req) {
        printf("%d\n", abort_list.size()); // 输出打断请求数
        for (int i = 0; i < abort_list.size(); ++i) {
            done_req.insert(abort_list[i]); // 将打断请求id存入已完成请求集合
            printf("%d\n", abort_list[i]); // 输出打断请求id
        }
    }

    // 删除操作打断未完成的请求
    bool abortRequest(int cur_time, std::vector<Disk> &disk_list, std::unordered_map<int, Object> &object_list, std::vector<int>& delete_obj_ids, Heatmap &heatmap, std::unordered_set<int>& done_req, Collection &collection) {
        
        int n_delete = delete_obj_ids.size();
        int sum_abort = 0;
        std::vector<int> abort_list;
        for(int i = 0; i < n_delete; ++i) {     // 遍历删除对象id
            int id = delete_obj_ids[i];         // 获取删除对象id
            Object &object = object_list[id];   //获取对象

            auto& req_map = object.getRequestMap(); //获取该对象请求列表
            sum_abort += req_map.size();        //累加打断请求数

            auto it = req_map.begin();      //遍历请求列表
            while(it != req_map.end()) {
                Request &req = it->second;  //获取请求

                abort_list.emplace_back(req.request_id);    // 将请求id存储到abort_list打断列表中

                // heatmap.request_delete(req, cur_time, object_list); // 更新热力图
                int object_id = req.object_id;
                int object_size = object_list[object_id].size;
                int value_lost = cur_time - req.timestamp;
                int last_value = (105 - value_lost) > 0 ? (105 - value_lost) * object_size : 0;
                int disk_id = object_list[object_id].disk[0];
                int tile_id = object_list[object_id].tiles[0][0].first;
                if(heatmap.heatmap[disk_id][tile_id] < last_value) {
                    heatmap.heatmap[disk_id][tile_id] = 0; // 更新热度图
                } else {
                    heatmap.heatmap[disk_id][tile_id] -= last_value; // 更新热度图
                }
                heatmap.num[disk_id][tile_id] -= object_size;   
    
                    
                disk_id = object_list[object_id].disk[1];
                tile_id = object_list[object_id].tiles[1][0].first;
                if(heatmap.heatmap[disk_id][tile_id] < last_value) {
                    heatmap.heatmap[disk_id][tile_id] = 0; // 更新热度图
                } else {
                    heatmap.heatmap[disk_id][tile_id] -= last_value; // 更新热度图
                }
                heatmap.num[disk_id][tile_id] -= object_size;   
    
                    
                disk_id = object_list[object_id].disk[2];
                tile_id = object_list[object_id].tiles[2][0].first;
                if(heatmap.heatmap[disk_id][tile_id] < last_value) {
                    heatmap.heatmap[disk_id][tile_id] = 0; // 更新热度图
                } else {
                    heatmap.heatmap[disk_id][tile_id] -= last_value; // 更新热度图
                }
                heatmap.num[disk_id][tile_id] -= object_size;   
                
                // for (int j = 0; j < REP_NUM; ++j) { // 遍历副本
                //     int disk_id = object.getDiskId(j); // 获取磁盘id
                //     Disk &disk = disk_list[disk_id]; // 获取磁盘对象
                //     int tile_id = object.getTiles(j)[0].first; // 获取对象在磁盘上的位置
                //     Tile &tile = disk.getTile(tile_id); // 获取磁盘页对象
                //     // tile.erase_requestmap(req.request_id); // 删除请求映射
                // }

                it = req_map.erase(it); // 删除请求    

            }
            if(popObject(object, disk_list)) { // 删除对象
                object_list.erase(id); // 从对象列表中删除对象
                collection.deleteGarbage(id, disk_list.size()); // 从垃圾回收列表中删除对象
            } else {
                throw std::runtime_error("删除对象失败！"); // 抛出异常
                return false; // 删除失败
            }

            
        }

        // ASSERT_DEBUG(sum_abort == abort_list.size(), "打断请求数不一致！\n", sum_abort, abort_list.size()) // 检查打断请求数是否一致
        output(abort_list, done_req); // 输出打断请求列表
    }




    // 处理删除请求
    void handleDeleteRequest(std::vector<Disk> &disk_list, std::unordered_map<int, Object> &object_list, int cur_time, Heatmap &heatmap, std::unordered_set<int>& done_req, Collection &collection) {
        int n_delete;
        scanf("%d", &n_delete);
        if(n_delete == 0) {
            printf("0\n");
            return;
        }
        std::vector<int> _ids(n_delete);
        for (int i = 0; i < n_delete; ++i) {
            scanf("%d", &_ids[i]);
        }

        abortRequest(cur_time, disk_list, object_list, _ids, heatmap, done_req, collection); // 删除操作打断未完成的请求
    }


};


#endif // DELETE_HPP

