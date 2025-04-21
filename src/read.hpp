#ifndef READ_HPP
#define READ_HPP

#include "util.hpp"
#include "request.hpp"
#include "slidewindow.hpp"
#include "threadpool.hpp"
// #pragma GCC optimize("O2")


extern int G, V, N;
class Disk_op {

public:

    void copy_heatmap(Heatmap &heatmap, Heatmap &fake_heatmap) {
        // 复制热力图
        for (int i = 0; i < heatmap.heatmap.size(); i++) {
            std::memcpy(fake_heatmap.heatmap[i].data(), heatmap.heatmap[i].data(), heatmap.heatmap[i].size() * sizeof(int));
            // for (int j = 0; j < heatmap.heatmap[i].size(); j++) {
            //     fake_heatmap.heatmap[i][j] = heatmap.heatmap[i][j];
            // }
        }
    }



    void read_until_simulate(int time, int id, Disk &disk, std::vector<std::vector<int>> &heatmap, std::unordered_map<int, Object> &object_list) {
        
        Point &point = disk.points[id];
        //磁头处于工作状态
        int point_now = point.point_location;
        int point_end = point.read_interval.second;
        
        // 获取当前磁头需要访问的块范围,需要处理结束位置
        int start_tile = point_now / disk.tile_size;
        int end_tile = point_end % disk.tile_size == 0? point_end / disk.tile_size - 1: point_end / disk.tile_size;
        
        for(int tileid = start_tile; tileid <= end_tile; ++tileid) {
            Tile &tile = disk.tiles[tileid];
            if(tile.free_block.size() == tile.max_size) continue;
            // threadPool.enqueue([&]() {
                std::unordered_set<int> ObjectId = tile.get_obj_ids();
                for(auto &obj_id : ObjectId) {
                    Object &object = object_list[obj_id];
                    auto &request_map_new = object.request_map;
                    // auto it = request_map_new.begin();
                    for( auto &[req_id, req] : request_map_new) {
                        if(req.timestamp <= time) {
                            int value_lost = time - req.timestamp;
                            int last_value = std::max(105 - value_lost, 0) * object.size;

                            int disk_ids[3] = {object.disk[0], object.disk[1], object.disk[2]};
                            int tile_ids[3] = {object.tiles[0][0].first, object.tiles[1][0].first, object.tiles[2][0].first};

                            for (int i = 0; i < 3; ++i) {
                                // if (fake_heatmap.heatmap[disk_ids[i]][tile_ids[i]] >= last_value) {
                                    heatmap[disk_ids[i]][tile_ids[i]] += last_value;
                                // } else {
                                //     fake_heatmap.heatmap[disk_ids[i]][tile_ids[i]] = 0;
                                // }
                            }
                            
                        }else {
                            break;
                        }
                        // it = request_map_new.erase(it);
                    }
                    // fake_heatmap.request_done_fake(object, time, object_list);
                    
                }
            // });
        }
        // threadPool.wait(); // 等待所有线程完成

            // for (int i = point_now; i < point_end; ++i) {
            //     int tileId = i / disk.getTileSize();
            //     Tile &tile = disk.getTile(tileId);
            //     if (tile.objList[i - tile.start_pos].first != 0) {
            //         //如果当前块上有对象，读取对象
            //         int object_id = tile.objList[i - tile.start_pos].first;
            //         if (ObjectId.find(object_id) == ObjectId.end()) {
            //             ObjectId.insert(object_id);
            //             Object &object = object_list[object_id];
            //             std::map<int, Request> request_map_new = object.request_map;
            //             for (auto &[request_id, request] : request_map_new) {
            //                 if (request.timestamp <= time) {
            //                     fake_heatmap.request_done(request, time, object_list);
            //                 }else break;
            //             }
            //         }
            //     }else continue;
            // }
    }

    void read_until_simulate_alone(int time, int id, Disk &disk, Heatmap& fake_heatmap, std::unordered_map<int, Object> &object_list, ThreadPool& threadPool) {
        
        Point &point = disk.points[id];
        //磁头处于工作状态
        int point_now = point.point_location;
        int point_end = point.read_interval.second;
        
        // 获取当前磁头需要访问的块范围,需要处理结束位置
        int start_tile = point_now / disk.tile_size;
        int end_tile = point_end % disk.tile_size == 0? point_end / disk.tile_size - 1: point_end / disk.tile_size;
        
        for(int tileid = start_tile; tileid <= end_tile; ++tileid) {
            Tile &tile = disk.tiles[tileid];
            if(tile.cap_size == tile.max_size) continue;
            threadPool.enqueue([&]() {
                std::unordered_set<int> ObjectId = tile.get_obj_ids();
                for(auto &obj_id : ObjectId) {
                    Object &object = object_list[obj_id];
                    auto &request_map_new = object.request_map;
                    // auto it = request_map_new.begin();
                    for( auto &[req_id, req] : request_map_new) {
                        if(req.timestamp <= time) {
                            int value_lost = time - req.timestamp;
                            int last_value = std::max(105 - value_lost, 0) * object.size;

                            int disk_ids[3] = {object.disk[0], object.disk[1], object.disk[2]};
                            int tile_ids[3] = {object.tiles[0][0].first, object.tiles[1][0].first, object.tiles[2][0].first};

                            for (int i = 0; i < 3; ++i) {
                                if (fake_heatmap.heatmap[disk_ids[i]][tile_ids[i]] >= last_value) {
                                    fake_heatmap.heatmap[disk_ids[i]][tile_ids[i]] -= last_value;
                                } else {
                                    fake_heatmap.heatmap[disk_ids[i]][tile_ids[i]] = 0;
                                }
                            }
                            
                        }else {
                            break;
                        }
                    }
                    
                }
            });
        }
        threadPool.wait(); // 等待所有线程完成
    }

    void read_until(std::vector<int> &g, int time, int id, Disk &disk, std::unordered_map<int, std::pair<int, int>> &done_req_map, std::unordered_set<int> &request_queue_done, std::string &opstr, std::unordered_map<int, Object> &object_list, std::vector<Disk>& disk_list) {
        Point &point = disk.getPoint(id);
        int start_pos = point.point_location;
        int end_pos = point.read_interval.second;
        int i = start_pos;
        for (i = start_pos; i < end_pos; ++i) {
            int result = -1;
            if (point.check_token(time, g, READ, result)) {
                int tileId = i / disk.tile_size;  // 磁盘页id
                Tile &tile = disk.getTile(tileId);
                
                point.update_token(time, g, READ);

                opstr += "r";
                if (tile.objList[i - tile.start_pos].first != 0) {
                    int object_id = tile.objList[i - tile.start_pos].first;
                    int block_id = tile.objList[i - tile.start_pos].second;
                    Object &object = object_list[object_id];
                    block_read(object, block_id, time, request_queue_done, done_req_map, object_list, disk_list);
                }else continue;
            }else {
                break;
            }
        }
        point.point_location = i;
        if (point.point_location == end_pos) {
            point.current_state = 0;
        }
    }

    void block_read(Object &object, int block_id, int time, std::unordered_set<int> &request_queue_done, std::unordered_map<int, std::pair<int, int>> &done_req_map, std::unordered_map<int, Object> &object_list, std::vector<Disk>& disk_list) {
        //读取对象
        object.block_to_timestamp[block_id] = time;

        if (object.block_to_timestamp.size() == object.size) {
            int min_time = 1e9;
            for (auto &block : object.block_to_timestamp) {
                min_time = std::min(min_time, block.second);
            }
            auto it = object.request_map.begin();
            while (it != object.request_map.end()) {
                int request_id = it->first;
                Request &request = it->second;
                if (request.timestamp <= min_time) {
                    request.is_done = true;
                    request_queue_done.insert(request.request_id);
                    
                    //object 维护请求列表
                    
                    //tile 维护请求列表
                    int object_id = object.object_id;
                    int object_size = object_list[object_id].size;
                    int value_lost = time - request.timestamp;
                    int last_value = (105 - value_lost) > 0 ? (105 - value_lost) * object_size : 0;
                    done_req_map[request_id].first += last_value;
                    done_req_map[request_id].second += object_size;
            
                }else {
                    break;
                }
                it = object.request_map.erase(it);
            }
        }

    }

    bool schedule(std::vector<int> &g, Disk &disk, int time, int id, Heatmap &fake_heatmap, std::unordered_map<int, Object> &object_list, std::string &opstr, bool &flag_jump) {
        Point &point = disk.points[id];
        slidewindow sw;
        sw.calculateHeatmap(disk, id, fake_heatmap);
        // point.read_interval = {0, 1};
        point.read_interval = sw.atomic_read;
        int now_pos = point.point_location;
        int new_x = point.read_interval.first;
        int distance = (new_x - now_pos + V) % V;
        if (distance == 0) return true;
        if (distance <= point.current_token) {
            int result = -1;
            while (point.check_token(time, g, PASS, result) && point.point_location != new_x) {
                opstr += "p";

                //do update token
                point.update_token(time, g, PASS);

                point.point_location = (point.point_location + 1) % V;
            }
            return true;
        }else {
            int result = -1;
            if (point.check_token(time, g, JUMP, result)) {
                printf("j %d\n", new_x + 1);
                flag_jump = true;

                point.update_token(time, g, JUMP);

                point.point_location = new_x;
                return true;
            }else {
                int result = -1;
                while (point.check_token(time, g, READ, result) && point.point_location != new_x) {
                    opstr += "r";

                   point.update_token(time, g, READ);
                    point.point_location = (point.point_location + 1) % V;
                }
                if (point.point_location == new_x) return true;
                else return false;
            }

        }
        
    }
};

class ReadRequest
{
private:
public:
    std::unordered_set<int> request_queue_done; //在这个时间片内完成的请求队列
    std::unordered_set<int> request_queue_timeout; //在这个时间片内超时的请求队列

    void Read(int time, Heatmap &heatmap, std::unordered_map<int, Object> &object_list, std::vector<Disk> &disk_list, std::unordered_map<int, int> &request_object, std::vector<std::vector<std::pair<int, int>>>& rank, std::vector<std::vector<double>>& tags_busy_rate) {
        //读入读请求
        int n_read;
        int request_id, object_id;
        scanf("%d", &n_read);
        for (int i = 1; i <= n_read; i++) {
            scanf("%d%d", &request_id, &object_id);




            /**
             * 加入rank跳过tag
             */

            std::vector<int> skip_tags;
            auto &obj = object_list[object_id];
            bool skip = false;
            for(int i = 0; i < SKIP_TAGS; ++i) {
                if(obj.tag == rank[time/FRE_PER_SLICING][i].first) {
                    //表示该对象被跳过
                    request_queue_timeout.insert(request_id);
                    skip = true;
                    break;
                }
            }
            if(skip) continue;
             
            //裁剪策略测试
            Object &object = object_list[object_id];
             
            // 裁剪策略测试
            if(time <= 86400) {
                if(tags_busy_rate[(time - 1)/TIME_WINDOW][obj.tag - 1] >= CUT_OFF) {
                    request_queue_timeout.insert(request_id);
                    skip = true;
                }
           }
                
            if(skip) {
                continue;
            }



            Request request(request_id, object_id, time);
            request_object.insert({request_id, object_id});


            if(time <= 86400)
                heatmap.tag_request_in[(time - 1)/TIME_WINDOW][object.tag - 1] += 1;





            //更新热力表
            int disk_id = object.disk[0];
            int tile_id = object.tiles[0][0].first;
            int object_size = object_list[object_id].size;
            heatmap.heatmap[disk_id][tile_id] += (105 * object_size);
            heatmap.num[disk_id][tile_id] += object_size;
            disk_list[disk_id].tiles[tile_id].request_map.push({request_id, std::make_pair(time, object_size)});

            
            disk_id = object.disk[1];
            tile_id = object.tiles[1][0].first;
            heatmap.heatmap[disk_id][tile_id] += (105 * object_size);
            heatmap.num[disk_id][tile_id] += object_size;
            disk_list[disk_id].tiles[tile_id].request_map.push({request_id, std::make_pair(time, object_size)});

            
            disk_id = object.disk[2];
            tile_id = object.tiles[2][0].first;
            heatmap.heatmap[disk_id][tile_id] += (105 * object_size);
            heatmap.num[disk_id][tile_id] += object_size;
            disk_list[disk_id].tiles[tile_id].request_map.push({request_id, std::make_pair(time, object_size)});




            //object 维护请求列表
            object.request_map.insert({request_id, request});


        }        
    }
    void Call_disk_read(int time, std::vector<Disk> &disk_list, std::unordered_map<int, Object> &object_list, Heatmap &heatmap
        , Heatmap &fake_heatmap, ThreadPool& threadPool, std::unordered_map<int, std::pair<int, int>> &done_req_map, std::vector<int> &g) {
        Disk_op disk_op;

        bool need_fake = false;
        for(int i = 0; i < N; ++i) {
            if(disk_list[i].points[0].current_state == 0 || disk_list[i].points[1].current_state == 0) {
                need_fake = true;
                break;
            }
        }

        if(need_fake) {
            disk_op.copy_heatmap(heatmap, fake_heatmap);
            std::vector<std::vector<std::vector<std::vector<int>>>> heatmap_list(N, std::vector<std::vector<std::vector<int>>>(POINT_NUM, std::vector<std::vector<int>>(N, std::vector<int>(disk_list[0].tile_nums, 0))));
            // std::vector<Heatmap> local_heatmaps(4, fake_heatmap);
            std::vector<std::pair<int, int>> heatmap_pair;
            for (int j = 0; j < N; ++j ) {
                Disk &disk = disk_list[j];
                for (int k = 0; k < POINT_NUM; ++k ) {
                    if(disk.points[k].current_state == 1) {
                        heatmap_pair.emplace_back(std::make_pair(j, k));
                        threadPool.enqueue([&, j, k]() {
                            disk_op.read_until_simulate(time, k, disk, heatmap_list[j][k], object_list);
                        });
                        // disk_op.read_until_simulate(time, k, disk, fake_heatmap, object_list, threadPool);
                        // threadPool.wait(); // 等待所有线程完成
                    }
                }
            }
            threadPool.wait(); // 等待所有线程完成

            // 合并所有临时 heatmap 到 fake_heatmap
            for (int i = 0; i < fake_heatmap.heatmap.size(); ++i) {
                std::vector<int> &tmp_heatmap = fake_heatmap.heatmap[i];
                threadPool.enqueue([&tmp_heatmap, &heatmap_list, i, &heatmap_pair]() {
                    for (int j = 0; j < tmp_heatmap.size(); ++j) {
                        for (const auto &[disk_id, point_id] : heatmap_pair) {
                            int delta = heatmap_list[disk_id][point_id][i][j];
                            if (delta > 0) {
                                tmp_heatmap[j] = std::max(0, tmp_heatmap[j] - delta);
                            }
                        }
                    }
                });
            }
            threadPool.wait(); // 等待所有线程完成
        }
         
    

        for (int i = 0; i < N; ++i ) {
            Disk &disk = disk_list[i];
            for (int j = 0; j < POINT_NUM; j ++ ) {
                std::string opstr = "";
                bool flag_jump = false;
                Point &point = disk.points[j];
                int idxx = std::ceil((double)time / 1800);
                point.current_token = G + g[idxx];
                if (point.current_state == 1) {
                    disk_op.read_until(g, time, j, disk, done_req_map, request_queue_done, opstr, object_list, disk_list);
                }
                
                while (point.current_state == 0 && disk_op.schedule(g, disk, time, j, fake_heatmap, object_list, opstr, flag_jump)) //真假表替换
                {
                    point.current_state = 1;
                    disk_op.read_until(g, time, j, disk, done_req_map, request_queue_done, opstr, object_list, disk_list);
                    disk_op.read_until_simulate_alone(time, j, disk, fake_heatmap, object_list, threadPool);
                }
                if (flag_jump == false) {
                    printf("%s#\n", opstr.c_str());
                    
                }
            }
            // printf("#\n");
        }
    }

    void Output(Heatmap &heatmap, int time, std::unordered_map<int, Object> &object_list, std::unordered_map<int, int>& request_object, std::unordered_set<int>& done_req) {
        
        printf("%d\n", request_queue_done.size());
        for (auto &req_id : request_queue_done) {
            // heatmap.request_done(request, time, object_list);
            done_req.insert(req_id);
            printf("%d\n", req_id);
        }
        
        request_queue_done.clear();
    }

    void output_timeout(std::unordered_map<int, int>& request_object, std::unordered_map<int, Object> &object_list, Heatmap &heatmap, int time) {
        printf("%d\n", request_queue_timeout.size());
        for (auto &req_id : request_queue_timeout) {
            printf("%d\n", req_id);
            if(request_object.find(req_id) == request_object.end()) {
                continue;
            }
            int obj_id = request_object[req_id];
            int tag = object_list[obj_id].tag;
            if(time >=105 && time !=864505)
                heatmap.tag_request_busy[(time - 105)/TIME_WINDOW][tag - 1] += 1;

            object_list[obj_id].request_map.erase(req_id);
        }
        request_queue_timeout.clear();

    }
};







#endif // READ_HPP