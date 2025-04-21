#ifndef WRITE_HPP
#define WRITE_HPP

#include "util.hpp"
#include "disk.hpp"
#include "object.hpp"
#include "collection.hpp"

class WriteRequest {
private:

public:
    int object_id;  // 对象id
    int size;   // 对象大小
    int tag;    // 对象tag
    std::vector<std::pair<int, std::vector<int>>> obj_point;  // 对象对应的磁盘物理位置
    std::vector<std::vector<std::pair<int, int>>> tilesinfo; //每个磁盘下存储的tile id以及偏移量

    // 构造函数
    WriteRequest(int object_id, int size, int tag) : object_id(object_id), size(size), tag(tag) {    }

    bool checkSaved() {
        return obj_point.size() == REP_NUM; // 检查是否已经存储到三个磁盘中
    }

    // 检查磁盘中是否存在该对象
    bool checkExist(int disk_id) {
        for(auto &point : obj_point) {
            if(point.first == disk_id) return true;
        }
        return false;
    }

    // 将对象封装到对象列表中
    bool packObject(std::unordered_map<int, Object> &object_list) {
        Object object(object_id, size, tag);
        object.packObject(std::move(tilesinfo), obj_point); // 打包对象
        object_list[object_id] = std::move(object); // 将对象存储到对象列表中
        return true;
    }

    // 将对象存储到磁盘中自己的tag 相邻 的磁盘页中(相融存储)
    int pushObjectNeibor(Disk &disk, int &tile_id) {
        std::pair<int, int> tile_range = disk.get_tile_range(tag);

        // 如果没有找到合适的 tile，进行二次探测：尝试存储到其他 tile 中
        // 二次探测：从 tile_range 的边界开始，向两侧扩展
        int left = tile_range.first - 1; // 左侧探测起点
        int right = tile_range.second + 1; // 右侧探测起点
        while(left >= 0 || right < disk.get_tile_nums()) {
            // 优先检查左侧
            if(left >= 0) {
                std::vector<int> block_ids = disk.insert_obj(object_id, size, tilesinfo, left);
                if(block_ids.size() == size) {
                    obj_point.emplace_back(make_pair(disk.get_id(), block_ids));
                    tile_id = left;
                    return disk.get_id();
                }
                --left; // 向左扩展
            }

            // 然后检查右侧
            if(right < disk.get_tile_nums()) {
                std::vector<int> block_ids = disk.insert_obj(object_id, size, tilesinfo, right);
                if(block_ids.size() == size) {
                    obj_point.emplace_back(make_pair(disk.get_id(), block_ids));
                    tile_id = right;
                    return disk.get_id();
                }
                ++right; // 向右扩展
            }
        }
        return -1; // 存储失败

    }

    // 将对象存储到磁盘中自己的tag 对应 的磁盘页中(请求存储)
    int pushObjectREQ(Disk &disk) {
        std::pair<int, int> &tile_range = disk.get_tile_range(tag); // 获取磁盘页范围
        if(tile_range == disk.get_default_range()) return -1; // 未找到该磁盘中tag对应的磁盘页

        // 在空闲磁盘页上插入对象
        for(int tile_id = tile_range.first; tile_id <= tile_range.second; ++tile_id) {
            std::vector<int> block_ids = disk.insert_obj(object_id, size, tilesinfo, tile_id);
            if(block_ids.size() == size) {
                obj_point.emplace_back(make_pair(disk.get_id(), block_ids)); // 存储对象对应的磁盘物理位置
                return disk.get_id(); // 返回磁盘id
            }
        }
        return -1; // 存储失败
    }

    // 将对象存储到磁盘中
    bool pushObject(std::vector<Disk> &disk_list, std::unordered_map<int, Object> &object_list, int cur_time, Collection &collection) {
        int rep_nums = 0; // 存储到的磁盘数量
        int disk_nums = disk_list.size(); // 磁盘实际数量
        int start = ((cur_time % FRE_PER_SLICING)) % disk_nums; // 从当前时间片开始的磁盘页
        int times = 0; // 搜索次数

        /**
         * 挤0,3,6号磁盘 存储策略
         */
        std::vector<int> main_disk_ids;
        main_disk_ids.emplace_back(0); // 主磁盘id
        main_disk_ids.emplace_back(3); // 主磁盘id
        main_disk_ids.emplace_back(6); // 主磁盘id
        // main_disk_ids.emplace_back(9); // 主磁盘id

        for(int main_i = 0; main_i < main_disk_ids.size(); ++main_i ) {
            if(rep_nums == REP_NUM) break;
            Disk &main_disk = disk_list[main_disk_ids[main_i]]; // 主磁盘
            if(!checkExist(main_disk.get_id()) && pushObjectREQ(main_disk) != -1) {
                ++rep_nums; // 存储到主磁盘中
                for(int sub_i = main_disk_ids[main_i] + 1; sub_i < disk_nums; ++sub_i) {
                    if(rep_nums == REP_NUM) break; // 已经存储到三个磁盘中
                    Disk &sub_disk = disk_list[sub_i]; // 副磁盘
                    if(!checkExist(sub_disk.get_id()) && pushObjectREQ(sub_disk) != -1) {
                        ++rep_nums; // 存储到副磁盘中
                    }
                }
            }
            
        }
        for(int start_disk = 0; times < disk_nums;) {
            Disk &disk = disk_list[start_disk];
            if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中

            if(!checkExist(disk.get_id())) {  // 判断磁盘是否包含tag对应的磁盘页，并且未存储过
                if(pushObjectREQ(disk) != -1) { // 将对象存储到磁盘中
                    ++rep_nums;
                }   
            }
            times += 1;
            // start_disk = (start_disk + 3) % disk_nums; // 循环遍历磁盘
            ++ start_disk;
        }

        if(rep_nums != REP_NUM) {
            // 未找到合适的磁盘页，需要存储在磁盘空闲区或者其他tag的tile中(相融存储)
            times = 0;
            // for(int start_disk = (start + 1) % disk_nums; times < disk_nums; (++start_disk) %= disk_nums) {
            for(int start_disk = 9; times < disk_nums;) {
                Disk &disk = disk_list[start_disk];
                if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中
                if(!checkExist(disk.id)) {  // 未存储过
                    int tile_id = -1;
                    if(pushObjectNeibor(disk, tile_id) != -1) { // 将对象存储到磁盘中
                        int cur_tag = -1;
                        for(int i = 1 ; i <= disk.tag_tiles.size(); ++i) { 
                            if(disk.tag_tiles[i].first <= tile_id && disk.tag_tiles[i].second >= tile_id) {
                                cur_tag = i; // 获取当前磁盘页的tag
                                break;
                            }
                        }
                        collection.insert(disk.id, tag, object_id, size, tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
                        ++rep_nums;
                    }   
                }
                times += 1;
                // start_disk = (start_disk + 3) % disk_nums; // 循环遍历磁盘
                -- start_disk;
            }
        }




        /**
         * 一主两副 存储策略
         */
        // std::vector<int> main_disk_ids; // 主磁盘id
        // main_disk_ids.emplace_back(0);
        // main_disk_ids.emplace_back(3);
        // main_disk_ids.emplace_back(6);

        
        // std::vector<int> sub_disk_ids;
        // sub_disk_ids.emplace_back(1);
        // sub_disk_ids.emplace_back(2);
        // sub_disk_ids.emplace_back(4);
        // sub_disk_ids.emplace_back(5);
        // sub_disk_ids.emplace_back(7);
        // sub_disk_ids.emplace_back(8);

        // for(int main_i = 0; main_i < main_disk_ids.size(); ++main_i) {
        //     if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中
        //     Disk &main_disk = disk_list[main_disk_ids[main_i]];
        //     int main_tile_id = -1;
        //     if(!checkExist(main_disk.get_id())) {  // 判断磁盘是否包含tag对应的磁盘页，并且未存储过
        //         if(pushObjectREQ(main_disk) != -1) { // 将对象存储到磁盘中
        //             ++rep_nums;
        //             for(int sub_i = 1; sub_i <= 2; ++sub_i) {
        //                 Disk &sub_disk = disk_list[main_disk_ids[main_i] + sub_i];
        //                 int sub_tile_id = -1;
        //                 if(pushObjectREQ(sub_disk) != -1) {
        //                     ++rep_nums;
        //                 }
        //                 // else if(pushObjectNeibor(sub_disk, sub_tile_id) != -1) { // 将副对象存储到副磁盘邻近tag中
        //                 //     int cur_tag = -1;
        //                 //     for(int i = 1 ; i <= sub_disk.tag_tiles.size(); ++i) { 
        //                 //         if(sub_disk.tag_tiles[i].first <= sub_tile_id && sub_disk.tag_tiles[i].second >= sub_tile_id) {
        //                 //             cur_tag = i; // 获取当前磁盘页的tag
        //                 //             break;
        //                 //         }
        //                 //     }
        //                 //     collection.insert(sub_disk.id, tag, object_id, size, sub_tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
        //                 //     ++rep_nums;
        //                 // }
        //             }
        //             if(rep_nums < REP_NUM) {
        //                 Disk &disk_nine = disk_list[9]; // 9号磁盘
        //                 int disk_nine_tile_id = -1;
        //                 if(pushObjectREQ(disk_nine) != -1) {
        //                     ++rep_nums;
        //                 }else if(pushObjectNeibor(disk_nine, disk_nine_tile_id) != -1) { // 将副对象存储到副磁盘邻近tag中
        //                     int cur_tag = -1;
        //                     for(int i = 1 ; i <= disk_nine.tag_tiles.size(); ++i) { 
        //                         if(disk_nine.tag_tiles[i].first <= disk_nine_tile_id && disk_nine.tag_tiles[i].second >= disk_nine_tile_id) {
        //                             cur_tag = i; // 获取当前磁盘页的tag
        //                             break;
        //                         }
        //                     }
        //                     collection.insert(disk_nine.id, tag, object_id, size, disk_nine_tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
        //                     ++rep_nums;
        //                 }
        //                 if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中

        //                 // 9号磁盘存储失败，尝试存储到其他副磁盘中
        //                 for(int sub_i = 0; sub_i < sub_disk_ids.size(); ++sub_i) {
        //                     if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中
        //                     Disk &sub_disk = disk_list[sub_disk_ids[sub_i]];
        //                     int sub_tile_id = -1;
        //                     bool save = !checkExist(sub_disk.get_id());
        //                     if(save && pushObjectREQ(sub_disk) != -1) {
        //                         ++rep_nums;
        //                     }else if(save && pushObjectNeibor(sub_disk, sub_tile_id) != -1) { // 将副对象存储到副磁盘邻近tag中
        //                         int cur_tag = -1;
        //                         for(int i = 1 ; i <= sub_disk.tag_tiles.size(); ++i) { 
        //                             if(sub_disk.tag_tiles[i].first <= sub_tile_id && sub_disk.tag_tiles[i].second >= sub_tile_id) {
        //                                 cur_tag = i; // 获取当前磁盘页的tag
        //                                 break;
        //                             }
        //                         }
        //                         collection.insert(sub_disk.id, tag, object_id, size, sub_tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
        //                         ++rep_nums;
        //                     }
        //                 }

        //             }
        //             ASSERT_DEBUG(rep_nums == REP_NUM, "存储失败！\n", rep_nums, object_id, size, tag, main_disk.id)
                    
        //         }
        //         // else if(pushObjectNeibor(main_disk, main_tile_id) != -1) { // 将主存对象存储到主磁盘邻近tag中
        //         //     int cur_tag = -1;
        //         //     for(int i = 1 ; i <= main_disk.tag_tiles.size(); ++i) { 
        //         //         if(main_disk.tag_tiles[i].first <= main_tile_id && main_disk.tag_tiles[i].second >= main_tile_id) {
        //         //             cur_tag = i; // 获取当前磁盘页的tag
        //         //             break;
        //         //         }
        //         //     }
        //         //     collection.insert(main_disk.id, tag, object_id, size, main_tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
        //         //     ++rep_nums;

        //         //     for(int sub_i = 1; sub_i <= 2; ++sub_i) {
        //         //         Disk &sub_disk = disk_list[main_disk_ids[main_i] + sub_i];
        //         //         int sub_tile_id = -1;
        //         //         if(pushObjectREQ(sub_disk) != -1) {
        //         //             ++rep_nums;
        //         //         }else if(pushObjectNeibor(sub_disk, sub_tile_id) != -1) { // 将副对象存储到副磁盘邻近tag中
        //         //             int cur_tag = -1;
        //         //             for(int i = 1 ; i <= sub_disk.tag_tiles.size(); ++i) { 
        //         //                 if(sub_disk.tag_tiles[i].first <= sub_tile_id && sub_disk.tag_tiles[i].second >= sub_tile_id) {
        //         //                     cur_tag = i; // 获取当前磁盘页的tag
        //         //                     break;
        //         //                 }
        //         //             }
        //         //             collection.insert(sub_disk.id, tag, object_id, size, sub_tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
        //         //             ++rep_nums;
        //         //         }
        //         //     }
        //         //     ASSERT_DEBUG(rep_nums == REP_NUM, "存储失败！\n", rep_nums, object_id, size, tag, main_disk.id)
        //         // }   
        //     }
        // }
        // // ASSERT_DEBUG(rep_nums == 0, "未完全存储在主副盘中！\n", rep_nums, object_id, size, tag)
        // if(rep_nums == 0) {
        //     main_disk_ids.clear();
        //     main_disk_ids.emplace_back(9);

        //     // 从当前时间片开始的磁盘页
        //     for(int main_i = 0; main_i < main_disk_ids.size(); ++main_i) {
        //         if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中

        //         Disk &main_disk = disk_list[main_disk_ids[main_i]];
        //         int main_tile_id = -1;
        //         if(pushObjectREQ(main_disk) != -1) { // 将对象存储到磁盘中
        //             ++rep_nums;
        //             int times = 0;
        //             for(int sub_i = cur_time%sub_disk_ids.size(); times < sub_disk_ids.size(); (++sub_i) %= sub_disk_ids.size()) {
        //                 if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中
        //                 Disk &sub_disk = disk_list[sub_disk_ids[sub_i]];
        //                 int sub_tile_id = -1;
        //                 if(pushObjectREQ(sub_disk) != -1) {
        //                     ++rep_nums;
        //                 }else if(pushObjectNeibor(sub_disk, sub_tile_id) != -1) { // 将副对象存储到副磁盘邻近tag中
        //                     int cur_tag = -1;
        //                     for(int i = 1 ; i <= sub_disk.tag_tiles.size(); ++i) { 
        //                         if(sub_disk.tag_tiles[i].first <= sub_tile_id && sub_disk.tag_tiles[i].second >= sub_tile_id) {
        //                             cur_tag = i; // 获取当前磁盘页的tag
        //                             break;
        //                         }
        //                     }
        //                     collection.insert(sub_disk.id, tag, object_id, size, sub_tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
        //                     ++rep_nums;
        //                 }
        //                 times += 1;
        //             }
        //             ASSERT_DEBUG(rep_nums == REP_NUM, "存储失败！\n", rep_nums, object_id, size, tag, main_disk.id)
                    
        //         }
        //         else if(pushObjectNeibor(main_disk, main_tile_id) != -1) { // 将主存对象存储到主磁盘邻近tag中
        //             int cur_tag = -1;
        //             for(int i = 1 ; i <= main_disk.tag_tiles.size(); ++i) { 
        //                 if(main_disk.tag_tiles[i].first <= main_tile_id && main_disk.tag_tiles[i].second >= main_tile_id) {
        //                     cur_tag = i; // 获取当前磁盘页的tag
        //                     break;
        //                 }
        //             }
        //             collection.insert(main_disk.id, tag, object_id, size, main_tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
        //             ++rep_nums;

        //             int times = 0;
        //             for(int sub_i = cur_time%sub_disk_ids.size(); times < sub_disk_ids.size(); (++sub_i) %= sub_disk_ids.size()) {
        //                 if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中
        //                 Disk &sub_disk = disk_list[sub_disk_ids[sub_i]];
        //                 int sub_tile_id = -1;
        //                 if(pushObjectREQ(sub_disk) != -1) {
        //                     ++rep_nums;
        //                 }else if(pushObjectNeibor(sub_disk, sub_tile_id) != -1) { // 将副对象存储到副磁盘邻近tag中
        //                     int cur_tag = -1;
        //                     for(int i = 1 ; i <= sub_disk.tag_tiles.size(); ++i) { 
        //                         if(sub_disk.tag_tiles[i].first <= sub_tile_id && sub_disk.tag_tiles[i].second >= sub_tile_id) {
        //                             cur_tag = i; // 获取当前磁盘页的tag
        //                             break;
        //                         }
        //                     }
        //                     collection.insert(sub_disk.id, tag, object_id, size, sub_tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
        //                     ++rep_nums;
        //                 }
        //                 times += 1;
        //             }
        //             ASSERT_DEBUG(rep_nums == REP_NUM, "存储失败！\n", rep_nums, object_id, size, tag, main_disk.id)
        //         }
        //     }

        // }


        /**
         * 挤0相融存储
         */
        // for(int start_disk = (start + 1) % disk_nums; times < disk_nums; (++start_disk) %= disk_nums) {
        // // for(int start_disk = 0; times < disk_nums;) {
        //     Disk &disk = disk_list[start_disk];
        //     if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中

        //     if(!checkExist(disk.get_id())) {  // 判断磁盘是否包含tag对应的磁盘页，并且未存储过
        //         if(pushObjectREQ(disk) != -1) { // 将对象存储到磁盘中
        //             ++rep_nums;
        //         }   
        //     }
        //     times += 1;
        //     // start_disk = (start_disk + 3) % disk_nums; // 循环遍历磁盘
        //     // ++ start_disk;
        // }

        // if(rep_nums != REP_NUM) {
        //     // 未找到合适的磁盘页，需要存储在磁盘空闲区或者其他tag的tile中(相融存储)
        //     times = 0;
        //     for(int start_disk = (start + 1) % disk_nums; times < disk_nums; (++start_disk) %= disk_nums) {
        //     // for(int start_disk = 0; times < disk_nums;) {
        //         Disk &disk = disk_list[start_disk];
        //         if(rep_nums == REP_NUM) break;  // 已经存储到三个磁盘中
        //         if(!checkExist(disk.id)) {  // 未存储过
        //             int tile_id = -1;
        //             if(pushObjectNeibor(disk, tile_id) != -1) { // 将对象存储到磁盘中
        //                 int cur_tag = -1;
        //                 for(int i = 1 ; i <= disk.tag_tiles.size(); ++i) { 
        //                     if(disk.tag_tiles[i].first <= tile_id && disk.tag_tiles[i].second >= tile_id) {
        //                         cur_tag = i; // 获取当前磁盘页的tag
        //                         break;
        //                     }
        //                 }
        //                 collection.insert(disk.id, tag, object_id, size, tile_id, cur_tag); // 将对象id添加到垃圾回收列表中
        //                 ++rep_nums;
        //             }   
        //         }
        //         times += 1;
        //         // start_disk = (start_disk + 3) % disk_nums; // 循环遍历磁盘
        //         // ++ start_disk;
        //     }
        // }

        ASSERT_DEBUG(rep_nums == REP_NUM, "存储失败！\n", rep_nums, object_id, size, tag)
        if(rep_nums == REP_NUM) packObject(object_list); // 打包对象
        return rep_nums == REP_NUM; // 返回是否存储成功
    }



    int getObjectId() const { return object_id; } // 获取对象id
    int getSize() const { return size; } // 获取对象大小
    std::vector<std::pair<int, std::vector<int>>>& getObjPoint() { return obj_point; } // 获取对象对应的磁盘物理位置

};



/**
 * 写请求列表类
 */
class WriteList {
private:
public:

    void output(WriteRequest &wreq) { 
        printf("%d\n", wreq.getObjectId()); // 输出存储结果
        for(auto &[j, block_ids] : wreq.getObjPoint()) { // 输出磁盘id和块id
            printf("%d", j + 1); // 磁盘id从1开始
            for(auto &block_id : block_ids) {
                printf(" %d", block_id + 1); // 块id从1开始
            }
            printf("\n");
        }

    }

    // 处理写请求
    void handleWriteRequest(std::vector<Disk> &disk_list, std::unordered_map<int, Object> &object_list, int cur_time, Collection &collection) {
        int n_write;
        scanf("%d", &n_write);

        std::unordered_map<int, std::vector<WriteRequest>> w_req;   // 写请求队列,按照tag分类
        int id, size, tag;
        for(int i = 1; i <= n_write; ++i) {
            scanf("%d%d%d", &id, &size, &tag);
            w_req[tag].emplace_back(WriteRequest(id, size, tag ));
        }

        for(auto &[tag_id, wreqlist] : w_req) { // 按照tag分类处理写请求

            for(auto &wreq : wreqlist) { // 处理每个tag下的写请求
                if(wreq.pushObject(disk_list, object_list, cur_time, collection)) { // 将对象存储到磁盘中并打包
                    if(wreq.checkSaved()) {
                        output(wreq); // 输出存储结果
                        continue;
                    }
                    // ASSERT_DEBUG(wreq.checkSaved(), "存储失败！\n", id, size, tag);
                }
            }
        }
    }

};

#endif // WRITE_HPP
