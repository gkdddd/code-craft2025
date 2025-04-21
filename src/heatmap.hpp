#ifndef HEATMAP_HPP
#define HEATMAP_HPP

#include "util.hpp"
#include "request.hpp"
#include "object.hpp"
#include "tile.hpp"
#include "disk.hpp"
#include "threadpool.hpp"
// #pragma GCC optimize("O2")


#include <fstream>
#include <iomanip>

class Heatmap
{
private:
public:
    std::vector<std::vector<int>> heatmap; // 热力图
    std::vector<std::vector<int>> num; //每次更新扣除的热度
    // std::vector<std::vector<int>> tag_busy_num; //每1800s每个tagbusy的次数
    int is_true; // 是否为假热力图

    std::vector<std::vector<int>> tag_request_in; //每1800s每个tag的请求次数
    std::vector<std::vector<int>> tag_request_busy; //每1800s每个tag的请求繁忙次数
    std::vector<std::vector<double>> tag_request_busy_rate; //每1800s每个tag的请求繁忙率
    
    int getHeat(int disk_id, int tile_id) {
        return heatmap[disk_id][tile_id];
    }
    
    void request_in(Request& req, std::unordered_map<int, Object>& object_list){
        int object_id = req.object_id;
        int object_size = object_list[object_id].getSize();
        int time = req.timestamp;
        int disk_id = object_list[object_id].getDiskId(0);
        int tile_id = object_list[object_id].getTiles(0)[0].first;
        heatmap[disk_id][tile_id] += (105 * object_size);
        num[disk_id][tile_id] += object_size;
            // if(disk_id == 2 && tile_id == 171){
            // if(is_true == 1)
            //     std::cerr << "disk_id: "<< disk_id<<" tile_id: "<< tile_id<<"request_in: " << req.request_id<<" heat = " << heatmap[disk_id][tile_id] << " num = " << num[disk_id][tile_id] << std::endl;
            // }
        // if(time <= 86399 && is_true == 1)
        //     tile_request_in[time/600][disk_id][tile_id] += 1;

        disk_id = object_list[object_id].getDiskId(1);
        tile_id = object_list[object_id].getTiles(1)[0].first;
        heatmap[disk_id][tile_id] += (105 * object_size);
        num[disk_id][tile_id] += object_size;
        // if(time <= 86399 && is_true == 1)
        //     tile_request_in[time/600][disk_id][tile_id] += 1;

        disk_id = object_list[object_id].getDiskId(2);
        tile_id = object_list[object_id].getTiles(2)[0].first;
        heatmap[disk_id][tile_id] += (105 * object_size);
        num[disk_id][tile_id] += object_size;
        // if(time <= 86399 && is_true == 1)
        //     tile_request_in[time/600][disk_id][tile_id] += 1;
    }

    void request_done(Request& req,int time, std::unordered_map<int, Object>& object_list){
        int object_id = req.object_id;
        int object_size = object_list[object_id].getSize();
        int value_lost = time - req.timestamp;
        int last_value = (105 - value_lost) > 0 ? (105 - value_lost) * object_size : 0;
        const auto& disk = object_list[object_id].disk;
        const auto& tile = object_list[object_id].tiles;
        int disk_id = disk[0];
        int tile_id = tile[0][0].first;
        if(heatmap[disk_id][tile_id] >= last_value)
            heatmap[disk_id][tile_id] -= (last_value);
        if(num[disk_id][tile_id] >= object_size)
            num[disk_id][tile_id] -= object_size;   
            // if(disk_id == 2 && tile_id == 171){
            // if(is_true == 1)
            //     std::cerr << "disk_id: "<< disk_id<<" tile_id: "<< tile_id<<"request_done:" << req.request_id<< "heat = " << heatmap[disk_id][tile_id] << " num = " << num[disk_id][tile_id] << "time = " << time << "req_id = "<< req.request_id<< std::endl;
            // }
        disk_id = disk[1];
        tile_id = tile[1][0].first;
        if(heatmap[disk_id][tile_id] >= last_value)
            heatmap[disk_id][tile_id] -= (last_value);
        if(num[disk_id][tile_id] >= object_size)
            num[disk_id][tile_id] -= object_size;
        
        disk_id = disk[2];
        tile_id = tile[2][0].first;
        if(heatmap[disk_id][tile_id] >= last_value)
            heatmap[disk_id][tile_id] -= (last_value);
        if(num[disk_id][tile_id] >= object_size)
            num[disk_id][tile_id] -= object_size;   
    }

    void request_delete(Request& req,int time, std::unordered_map<int, Object>& object_list){
        int object_id = req.object_id;
        int object_size = object_list[object_id].getSize();
        int value_lost = time - req.timestamp;
        int last_value = (105 - value_lost) > 0 ? (105 - value_lost) * object_size : 0;
        int disk_id = object_list[object_id].getDiskId(0);
        int tile_id = object_list[object_id].getTiles(0)[0].first;
        if(heatmap[disk_id][tile_id] >= last_value)
            heatmap[disk_id][tile_id] -= (last_value);
        if(num[disk_id][tile_id] >= object_size)
            num[disk_id][tile_id] -= object_size;   
            // if(disk_id == 2 && tile_id == 171){
            // if(is_true == 1)
            //     std::cerr << "disk_id: "<< disk_id<<" tile_id: "<< tile_id<<"request_done:" << req.request_id<< "heat = " << heatmap[disk_id][tile_id] << " num = " << num[disk_id][tile_id] << "time = " << time << "req_id = "<< req.request_id<< std::endl;
            // }
        disk_id = object_list[object_id].getDiskId(1);
        tile_id = object_list[object_id].getTiles(1)[0].first;
        if(heatmap[disk_id][tile_id] >= last_value)
            heatmap[disk_id][tile_id] -= (last_value);
        if(num[disk_id][tile_id] >= object_size)
            num[disk_id][tile_id] -= object_size;
        
        disk_id = object_list[object_id].getDiskId(2);
        tile_id = object_list[object_id].getTiles(2)[0].first;
        if(heatmap[disk_id][tile_id] >= last_value)
            heatmap[disk_id][tile_id] -= (last_value);
        if(num[disk_id][tile_id] >= object_size)
            num[disk_id][tile_id] -= object_size;   
    }

    inline void request_busy(Request& req,int time, std::unordered_map<int, Object>& object_list){
        int object_id = req.object_id;
        int object_size = object_list[object_id].getSize();
        int value_lost = time - req.timestamp;
        int last_value = (105 - value_lost) > 0 ? (105 - value_lost) * object_size : 0;
        int disk_id = object_list[object_id].getDiskId(0);
        int tile_id = object_list[object_id].getTiles(0)[0].first;
        if(heatmap[disk_id][tile_id] >= last_value)
            heatmap[disk_id][tile_id] -= (last_value);
        if(num[disk_id][tile_id] >= object_size)
            num[disk_id][tile_id] -= object_size;
        // if(time >=106 && is_true == 1)
        //     tile_request_busy[(time - 105)/600][disk_id][tile_id] += 1;
            // if(disk_id == 2 && tile_id == 171){
            // if(is_true == 1)
            //     std::cerr << "disk_id: "<< disk_id<<" tile_id: "<< tile_id<<"request_done:" << req.request_id<< "heat = " << heatmap[disk_id][tile_id] << " num = " << num[disk_id][tile_id] << "time = " << time << "req_id = "<< req.request_id<< std::endl;
            // }
        disk_id = object_list[object_id].getDiskId(1);
        tile_id = object_list[object_id].getTiles(1)[0].first;
        if(heatmap[disk_id][tile_id] >= last_value)
            heatmap[disk_id][tile_id] -= (last_value);
        if(num[disk_id][tile_id] >= object_size)
            num[disk_id][tile_id] -= object_size;
        // if(time >=106 && is_true == 1)
        //     tile_request_busy[(time - 105)/600][disk_id][tile_id] += 1;
        
        disk_id = object_list[object_id].getDiskId(2);
        tile_id = object_list[object_id].getTiles(2)[0].first;
        if(heatmap[disk_id][tile_id] >= last_value)
            heatmap[disk_id][tile_id] -= (last_value);
        if(num[disk_id][tile_id] >= object_size)
            num[disk_id][tile_id] -= object_size;
        // if(time >=106 && is_true == 1)
        //     tile_request_busy[(time - 105)/600][disk_id][tile_id] += 1;

        // if(is_true == 1){
        //         int time_index = (time / 1800) % 48;
        //         tag_busy_num[object_list[object_id].getTag() - 1][time_index]++;
        // }
    }

    void update_heat(std::unordered_set<int>& request_queue_timeout,int time, std::unordered_map<int, Object>& object_list, std::vector<Disk>& disk_list, std::unordered_map<int, int>& request_object, ThreadPool& threadPool, std::unordered_set<int>& done_req_set, std::unordered_map<int, std::pair<int, int>>& done_req_map){

        for(auto &[req_id, heatinfo] : done_req_map) {
            int obj_id = request_object[req_id];
            Object &object = object_list[obj_id];
            int disk_id = object.disk[0];
            int tile_id = object.tiles[0][0].first;
            if (heatmap[disk_id][tile_id] < heatinfo.first)
                heatmap[disk_id][tile_id] = 0;
            else
                heatmap[disk_id][tile_id] -= heatinfo.first;
            num[disk_id][tile_id] -= heatinfo.second;
            
            disk_id = object.disk[1];
            tile_id = object.tiles[1][0].first;
            if (heatmap[disk_id][tile_id] < heatinfo.first)
                heatmap[disk_id][tile_id] = 0;
            else
                heatmap[disk_id][tile_id] -= heatinfo.first;
            num[disk_id][tile_id] -= heatinfo.second;
            
            disk_id = object.disk[2];
            tile_id = object.tiles[2][0].first;
            if (heatmap[disk_id][tile_id] < heatinfo.first)
                heatmap[disk_id][tile_id] = 0;
            else
                heatmap[disk_id][tile_id] -= heatinfo.first;
            num[disk_id][tile_id] -= heatinfo.second;
        }


        
        // threadPool.wait(); // 等待所有线程完成
        
        done_req_map.clear();




        std::vector<std::unordered_set<int>> timeout_set(heatmap.size());
        for(int i = 0; i < heatmap.size(); ++i){
            // threadPool.enqueue([i, &timeout_set, this, &disk_list, &done_req_set, time, &object_list, &request_object]() {
                Disk& disk = disk_list[i];
                std::unordered_set<int> &local_timeout_set = timeout_set[i];
                for(int j = 0; j < heatmap[0].size(); ++j){
                    Tile& tile = disk.tiles[j];
                    ASSERT_DEBUG(heatmap[i][j] >= 0, "热力图<0错误", i, j, heatmap[i][j], num[i][j])
                    ASSERT_DEBUG(heatmap[i][j] <= 105 * num[i][j], "热力图过大错误", i, j, heatmap[i][j], num[i][j])
                    auto& req_map = tile.request_map;
                    while(req_map.size()){
                        auto it = req_map.front();
                        if(it.second.first <= time - 104) {
                            if( done_req_set.count(it.first)) {
                                req_map.pop();
                                continue;
                            }else {
                                // ASSERT_DEBUG(request.request_id != -1, "获取request失败", it->first, object.getObjectId())
                                local_timeout_set.insert(it.first);
                                int object_size = it.second.second;
                                int object_id = request_object[it.first];
                                Object &object = object_list[object_id];
                                int k;
                                for(k =0;k<REP_NUM;k++){
                                    if(object.disk[k] == i)
                                    break;
                                }
                                int tile_id = object.tiles[k][0].first;
                                if(heatmap[i][tile_id] >= object_size)
                                    heatmap[i][tile_id] -= (object_size);
                                else
                                    heatmap[i][tile_id] = 0;
                                num[i][tile_id] -= object_size;
                            } 
                            // else {
                            //     // ASSERT_DEBUG(request.request_id != -1, "获取request失败", it->first, object.getObjectId())
                            //     local_timeout_set.insert(it.first);
                            //     auto &object = object_list[request_object[it.first]];
                            //     int object_size = it.second.second;
                            //     heatmap[i][j] -= (object_size);
                            //     num[i][j] -= object_size;   
                            // } 
                            req_map.pop();
                        }else {
                            break;
                        }
                    }
                    if (heatmap[i][j] < num[i][j]) 
                        heatmap[i][j] = 0; // 确保热力图不小于0
                    else
                        heatmap[i][j] -= num[i][j];
                }
            // });
        }
        // threadPool.wait(); // 等待所有线程完成

        for(const auto& localset : timeout_set) {
            request_queue_timeout.insert(localset.begin(), localset.end());
        }

    }


    // void request_done_fake(Object& object, int time, std::unordered_map<int, Object> &object_list) {
    //     int object_id = object.getObjectId();
    //     int object_size = object.getSize();
    //     int last_value_all = 0;
    //     auto &request_map_new = object.request_map;

    //     for (auto &request : request_map_new) {
    //         if (request.timestamp <= time) {
    //             int value_lost = time - request.timestamp;
    //             int last_value = (105 - value_lost) > 0 ? (105 - value_lost) * object_size : 0;
    //             last_value_all += last_value;                
    //         }else break;
    //     }

    //     int disk_id = object_list[object_id].getDiskId(0);
    //     int tile_id = object_list[object_id].getTiles(0)[0].first;
    //     if(heatmap[disk_id][tile_id] >= last_value_all)
    //         heatmap[disk_id][tile_id] -= (last_value_all);
    //     num[disk_id][tile_id] -= object_size;  
            
            
    //     disk_id = object_list[object_id].getDiskId(1);
    //     tile_id = object_list[object_id].getTiles(1)[0].first;
    //     if(heatmap[disk_id][tile_id] >= last_value_all)
    //         heatmap[disk_id][tile_id] -= (last_value_all);
    //     num[disk_id][tile_id] -= object_size;  

            
    //     disk_id = object_list[object_id].getDiskId(2);
    //     tile_id = object_list[object_id].getTiles(2)[0].first;
    //     if(heatmap[disk_id][tile_id] >= last_value_all)
    //         heatmap[disk_id][tile_id] -= (last_value_all);
    //     num[disk_id][tile_id] -= object_size;  
    // }

    // void output_tag_busy_num(){//列优先输出
    //     for(int i = 0; i < tag_busy_num[0].size(); ++i){
    //         std::cerr << "time: "<< i << std::endl;
    //         for(int j = 0; j < tag_busy_num.size(); ++j){
    //             std::cerr << "tag: "<< j << " busy num: " << tag_busy_num[j][i] << " ";
    //         }
    //         std::cerr << std::endl;
    //     }   
    // }
    void caculate_tag_request_busy_rate_and_save(const std::string &filename = "tag_request_busy_rate") {
        int x = tag_request_busy.size();
        int y = tag_request_busy[0].size();
    
        // 计算 busy rate
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if (tag_request_in[i][j] != 0)
                    tag_request_busy_rate[i][j] = static_cast<double>(tag_request_busy[i][j]) / static_cast<double>(tag_request_in[i][j]);
                else
                    tag_request_busy_rate[i][j] = 1.0;
            }
        }
    
        // 保存为 .dat 文件（纯文本，方便下次加载）
        std::ofstream dat_out(filename + ".dat");
        if (dat_out.is_open()) {
            dat_out << x << " " << y << "\n";
            for (int i = 0; i < x; ++i) {
                for (int j = 0; j < y; ++j) {
                    dat_out << std::setprecision(10) << tag_request_busy_rate[i][j] << " ";
                }
                dat_out << "\n";
            }
            dat_out.close();
        } else {
            std::cerr << "Failed to open " << filename << " for writing.\n";
        }
    }
};





#endif // HEATMAP_HPP