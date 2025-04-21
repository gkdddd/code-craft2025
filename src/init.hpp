#ifndef INIT_HPP
#define INIT_HPP
#include "util.hpp"
#include "disk.hpp"
// #pragma GCC optimize("O2")


class Init
{
public:
    void initialize(std::vector<std::vector<int>>& tags_delete, std::vector<std::vector<int>>& tags_save, std::vector<std::vector<int>>& tags_freq, std::vector<Disk>& disk_list, int M, int N,int T,int V,std::vector<std::pair<int,int>>& disk_rank){

        for(int i = 0;i<N;i++){
            Disk disk(i,V);
            disk_list.push_back(disk);
        }


        std::vector<int> tags_num = allocateDisk(disk_list[0].get_max_size(), tags_delete, tags_save, N);
        
        
        // tags_num[0] = 0; // 下标0不使用
        // tags_num[1] = 9; // 下标1使用
        // tags_num[2] += 4; // 下标2使用
        // tags_num[3] = 18; // 下标3使用
        // tags_num[4] = 7; // 下标4使用
        // tags_num[5] = 5; // 下标5使用 
        // tags_num[6] = 13; // 下标6使用
        // tags_num[7] = 7; // 下标7使用
        // tags_num[8] = 16; // 下标8使用
        // tags_num[9] = 23; // 下标9使用 
        // tags_num[10] = 22; // 下标10使用
        // tags_num[11] = 22; // 下标11使用
        // tags_num[12] = 8; // 下标12使用 
        // tags_num[13] = 8; // 下标13使用 
        // tags_num[14] = 17; // 下标14使用
        // tags_num[15] = 6; // 下标15使用 
        // tags_num[16] = 22; // 下标16使用





            //N个时间段的M个标签的频率最大值
        std::vector<std::vector<int>> matrix(N + 1, std::vector<int>(M + 1, 0));

        if ((((T - 1) / FRE_PER_SLICING) + 1) >= N) {

            int total_units = ((T - 1) / FRE_PER_SLICING) + 1 - IGNO_TIME;
            int length = total_units / N;
            int extra = total_units % N;
        
            // 计算最大值 matrix：按时间片分组
            for (int i = 1; i <= N; i++) {
                int start = (i - 1) * length + 1 + IGNO_TIME;
                int end = (i == N) ? (i * length + extra) : (i * length) + IGNO_TIME;
        
                // 对于第 i 组时间片，遍历每个标签，取该组时间片内的最大值
                for (int j = 1; j <= M; j++) {
                    auto max_it = std::max_element(tags_freq[j].begin() + start, tags_freq[j].begin() + end);
                    matrix[i][j] = *max_it;
                }

                disk_rank.emplace_back(std::make_pair<int,int>((start - 1 - IGNO_TIME) * 1800,(end -1 - IGNO_TIME)*1800));
            }
        } else {
            // 时间片数少于 N
            // 将 N 个磁盘平均分配到较少的时间片上：
            int total_time_slices = ((T - 1) / FRE_PER_SLICING) + 1; // 实际时间片数
            int groupSize = N / total_time_slices;      // 每个时间片分得的磁盘数的基础值
            int extra = N % total_time_slices;            // 前 extra 个时间片多分得1个磁盘
        
            int disk_row_index = 1; // matrix 行索引（1-based）
            for (int t = 1; t <= total_time_slices; t++) {
                // 当前时间片 t 对应的磁盘行数（组内行数）
                int groupCount = groupSize + (t <= extra ? 1 : 0);
                // 对于每个标签 j，在时间片 t 内，直接取 tags_freq[j][t] 作为该时间片的值（因为只有一个时间单位）
                for (int k = 0; k < groupCount; k++) {
                    for (int j = 1; j <= M; j++) {
                        matrix[disk_row_index][j] = tags_freq[j][t];
                    }
                    disk_row_index++;
                }
            }
        }

        for(int i = 1; i<=N; i++){
            int disk_pointer = 0;
            std::vector<int> rank = getTagRankingAtTime(matrix, i, M);
            // std::vector<int> rank = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
            for(int j = 1; j<= M; j++){
                    disk_list[i-1].set_tile_range(rank[j], disk_pointer, disk_pointer + tags_num[rank[j]] - 1);
                    disk_pointer = disk_pointer + tags_num[rank[j]];
            }
        }

        // disk_list[0].tag_tiles.insert({8,{0,27}});
        // disk_list[0].tag_tiles.insert({2,{28,52}});
        // disk_list[0].tag_tiles.insert({1,{53,62}});
        // disk_list[0].tag_tiles.insert({3,{63,75}});
        // disk_list[0].tag_tiles.insert({7,{76,85}});

        // disk_list[1].tag_tiles.insert({8,{0,27}});
        // disk_list[1].tag_tiles.insert({2,{28,52}});
        // disk_list[1].tag_tiles.insert({1,{53,62}});
        // disk_list[1].tag_tiles.insert({3,{63,75}});
        // disk_list[1].tag_tiles.insert({5,{76,81}});
        // disk_list[1].tag_tiles.insert({7,{82,91}});

        // disk_list[2].tag_tiles.insert({8,{0,27}});
        // disk_list[2].tag_tiles.insert({2,{28,52}});
        // disk_list[2].tag_tiles.insert({1,{53,62}});
        // disk_list[2].tag_tiles.insert({3,{63,75}});
        // disk_list[2].tag_tiles.insert({5,{76,81}});
        // disk_list[2].tag_tiles.insert({7,{82,91}});

        // disk_list[3].tag_tiles.insert({8,{0,27}});
        // disk_list[3].tag_tiles.insert({2,{28,52}});
        // disk_list[3].tag_tiles.insert({5,{64,69}});
        // disk_list[3].tag_tiles.insert({11,{58,63}});
        // disk_list[3].tag_tiles.insert({7,{70,79}});
        // disk_list[3].tag_tiles.insert({9,{53,57}});

        // disk_list[4].tag_tiles.insert({8,{0,27}});
        // disk_list[4].tag_tiles.insert({12,{28,53}});
        // disk_list[4].tag_tiles.insert({9,{54,58}});
        // disk_list[4].tag_tiles.insert({6,{59,61}});
        // disk_list[4].tag_tiles.insert({14,{62,67}});
        // disk_list[4].tag_tiles.insert({7,{68,77}});

        // disk_list[5].tag_tiles.insert({8,{0,27}});
        // disk_list[5].tag_tiles.insert({12,{28,53}});
        // disk_list[5].tag_tiles.insert({9,{54,58}});
        // disk_list[5].tag_tiles.insert({6,{59,61}});
        // disk_list[5].tag_tiles.insert({11,{62,67}});
        // disk_list[5].tag_tiles.insert({7,{68,77}});

        // disk_list[6].tag_tiles.insert({12,{0,25}});
        // disk_list[6].tag_tiles.insert({15,{26,37}});
        // disk_list[6].tag_tiles.insert({16,{38,57}});
        // disk_list[6].tag_tiles.insert({6,{58,60}});
        // disk_list[6].tag_tiles.insert({11,{61,66}});
        // disk_list[6].tag_tiles.insert({13,{67,72}});
        // disk_list[6].tag_tiles.insert({14,{73,78}});

        // disk_list[7].tag_tiles.insert({12,{0,25}});
        // disk_list[7].tag_tiles.insert({9,{26,30}});
        // disk_list[7].tag_tiles.insert({15,{31,42}});
        // disk_list[7].tag_tiles.insert({16,{43,62}});
        // disk_list[7].tag_tiles.insert({10,{63,72}});
        // disk_list[7].tag_tiles.insert({4,{73,85}});
        // disk_list[7].tag_tiles.insert({13,{86,91}});

        // disk_list[8].tag_tiles.insert({12,{0,25}});
        // disk_list[8].tag_tiles.insert({9,{26,30}});
        // disk_list[8].tag_tiles.insert({15,{31,42}});
        // disk_list[8].tag_tiles.insert({16,{43,62}});
        // disk_list[8].tag_tiles.insert({10,{63,72}});
        // disk_list[8].tag_tiles.insert({4,{73,85}});
        // disk_list[8].tag_tiles.insert({14,{86,91}});

        // disk_list[9].tag_tiles.insert({12,{0,25}});
        // disk_list[9].tag_tiles.insert({9,{26,30}});
        // disk_list[9].tag_tiles.insert({15,{31,42}});
        // disk_list[9].tag_tiles.insert({16,{43,62}});
        // disk_list[9].tag_tiles.insert({10,{63,72}});
        // disk_list[9].tag_tiles.insert({4,{73,85}});
        // disk_list[9].tag_tiles.insert({13,{86,91}});
    }

    std::vector<int> allocateDisk(int disk_size, const std::vector<std::vector<int>>& tags_delete, const std::vector<std::vector<int>>& tags_save, int disk_nums) {
        // 假设 tags_num[0] 不使用，M 为标签数
        std::vector<int> allocation(tags_delete.size(), 0); // 分配结果，大小为 M+1，下标0置为0

        std::vector<std::vector<int>> presum_add_delete(tags_delete.size(), std::vector<int>(tags_delete[0].size(), 0)); // 前缀和数组
        std::vector<int> max_save(tags_delete.size(), 0); // 每个标签的最大保存值
        for(int i = 1; i < tags_delete.size(); ++i) {
            for(int j = 1; j < tags_delete[0].size(); ++j) {
                presum_add_delete[i][j] = presum_add_delete[i][j-1] - tags_delete[i][j] + tags_save[i][j];
                max_save[i] = std::max(max_save[i], presum_add_delete[i][j]);
            }
        }
        
        int sum = std::accumulate(max_save.begin(), max_save.end(), 0); // 所有标签的最大保存值之和
        int sum_tile = disk_size * disk_nums / TILE_SIZE;

        for(int i = 1; i < tags_delete.size(); ++i) {
            allocation[i] = max_save[i] * sum_tile / sum / disk_nums;
        }



        
        return allocation;
    }

    std::vector<int> getTagRankingAtTime(const std::vector<std::vector<int>>& matrix, int time_idx, int M) {
        // 构造候选 vector，存放 pair<标签编号, 值>
        std::vector<std::pair<int, int>> candidates;
        for (int tag = 1; tag <= M; tag++) {
            candidates.emplace_back(tag, matrix[time_idx][tag]);
        }
        // 按照值从大到小排序
        std::sort(candidates.begin(), candidates.end(), [](const std::pair<int,int>& a, const std::pair<int,int>& b) {
            return a.second > b.second;
        });
        // 构造返回结果，大小为 M+1，下标0置为0，其余为排序后的标签编号
        std::vector<int> ranking(M + 1, 0);
        ranking[0] = 0; // 保持第0位为0
        for (int i = 0; i < candidates.size(); i++) {
            ranking[i + 1] = candidates[i].first;
        }
        return ranking;
    }

    void init_heatmap(Heatmap& heat, int disk_nums, int tile_nums,int flag) {
        heat.heatmap.resize(disk_nums, std::vector<int>(tile_nums, 0)); // 初始化热度图
        heat.num.resize(disk_nums, std::vector<int>(tile_nums, 0)); // 初始化热度图
        heat.is_true = flag; // 设置热度图是否真实
        
        heat.tag_request_in.resize(86400 / TIME_WINDOW, std::vector<int>(16, 0)); // 初始化请求次数
        heat.tag_request_busy.resize(86400 / TIME_WINDOW, std::vector<int>(16, 0)); // 初始化请求繁忙次数
        heat.tag_request_busy_rate.resize(86400 / TIME_WINDOW, std::vector<double>(16, 0)); // 初始化请求繁忙率
    }
    
};



#endif