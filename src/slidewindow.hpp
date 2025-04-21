#ifndef SLIDEWINDOW_HPP
#define SLIDEWINDOW_HPP
#include "util.hpp"
#include "disk.hpp"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include "heatmap.hpp"

#pragma GCC optimize("O2")

class slidewindow {
private:

    struct Precalc {
        std::vector<long long> S;     // 前缀和：S[i] = heat[0] + ... + heat[i-1]
        std::vector<long long> S_aj;  // 加权前缀和：S_aj[i] = 0*heat[0] + 1*heat[1] + ... + (i-1)*heat[i-1]
        std::vector<double> term;     // term[i] = S[i]*(1 - per_ratio * i) - per_ratio * S_aj[i]
    };

    // 根据 heatmap_fake 构造密集数组，并预计算 S、S_aj 与 term
    inline Precalc precompute(double per_ratio,Heatmap& fake_heatmap, int disk_id) {
        Precalc pc;
        const int n = fake_heatmap.heatmap[0].size(); // 磁盘页数
        pc.S.resize(n+1, 0);
        pc.S_aj.resize(n+1, 0);
        pc.term.resize(n+1, 0);

        // 计算前缀和与加权前缀和，同时预处理 term
        for (int i = 0; i < n; ++i) {
            pc.S[i+1] = pc.S[i] + fake_heatmap.heatmap[disk_id][i];
            // 计算加权前缀和时，注意 heat[i] 是从 0 开始的，所以要乘以 i
            pc.S_aj[i+1] = pc.S_aj[i] + i * fake_heatmap.heatmap[disk_id][i];
            // term[i] 只需计算到 i = n-1，最后一项单独处理
            pc.term[i] = pc.S[i] * (1 + per_ratio * i) - per_ratio * pc.S_aj[i];
        }
        pc.term[n] = pc.S[n] * (1 - per_ratio * n) - per_ratio * pc.S_aj[n];
        return pc;
    }

    // 三分搜索：在区间 [L, R] 内寻找整数 k，使得函数 f(k) 最大
    // 返回 (最大值, 对应的 k)
    template<typename Func>
    std::pair<double, int> ternarySearch(int L, int R, Func f) {
        while (R - L > 2) { // 控制精度
            int m1 = L + (R - L) / 3;
            int m2 = R - (R - L) / 3;
            double f1 = f(m1);
            double f2 = f(m2);
            if (f1 < f2)
                L = m1;
            else
                R = m2;
        }
        double bestVal = -std::numeric_limits<double>::infinity();
        int bestK = L;
        for (int k = L; k <= R; ++k) {
            double val = f(k);
            if (val > bestVal) {
                bestVal = val;
                bestK = k;
            }
        }
        return {bestVal, bestK};
    }

public:
    std::pair<int, int> atomic_read; // 原子读开始位置和结束物理位置

    bool calculateHeatmap(Disk &disk, int point_id, Heatmap &fake_heatmap){
        const int n = fake_heatmap.heatmap[0].size(); // 磁盘页数
        if (n == 0) return false;

        int window_punish = 0;
        int sch_const = 0;
        if(point_id == 0){
            window_punish = WINDOW_PUNISH_1;
            sch_const = SCH_CONST_1;
        }
        else{
            window_punish = WINDOW_PUNISH_2;
            sch_const = SCH_CONST_2;
        }
            
    
        const double per_ratio = window_punish / 100.0;
        double perpage_consume = TILE_SIZE / 16.0;
    
        Precalc pc = precompute(per_ratio, fake_heatmap, disk.get_id());
    
        double max_score = -std::numeric_limits<double>::infinity();
        int best_i = 0, best_k = 0;

        for (int i = 0; i < n; ++i) {
            // if (i >= exclude_start && i <= exclude_end) continue; // 跳过不允许的范围
    
            auto scoreFunc = [i, &pc, n, perpage_consume, per_ratio, sch_const](int k) -> double {
                if (k < 1 || i + k > n) return -std::numeric_limits<double>::infinity();
                
                double S = pc.term[i + k] - pc.term[i] - per_ratio * k * pc.S[i + k];
                return S / (k * perpage_consume + sch_const);
            };
    
            int max_k = n - i;
            if (max_k < 1) continue;
    
            auto [score, k] = ternarySearch(1, max_k, scoreFunc);
    
            if (score > max_score) {
                // int window_start = i;
                // int window_end = i + k - 1;
                
                // // 确保窗口不包含 [exclude_start, exclude_end]
                // if (window_end < exclude_start || window_start > exclude_end) {  
                    max_score = score;
                    best_i = i;
                    best_k = k;
                // }
            }
        }
    
        atomic_read.first = best_i * TILE_SIZE;
        atomic_read.second = std::min((best_i + best_k) * TILE_SIZE, disk.tile_nums * TILE_SIZE);
    
        return true;
    }

    
    
};

#endif
