#ifndef POINT_HPP
#define POINT_HPP

#include "util.hpp"
extern int G;
class Point
{
private:
    /* data */
public:

    int id;         // 磁头id
    int point_location; // 磁头位置
    int last_action; // 1: jump, 2: pass, 3: read
    int pre_token;   // 上一次操作的token
    int current_token; // 当前token
    int current_state; // 0: idle, 1: working

    std::pair<int, int> read_interval = {0, 0}; // 读操作的区间，闭区间

    Point(int id, int point_location, int last_action, int pre_token, int current_token, int current_state)
    {
        this->id = id;
        this->point_location = point_location;
        this->last_action = last_action;
        this->pre_token = pre_token;
        this->current_token = current_token;
        this->current_state = current_state;
    }

    Point() {
        id = -1;
        point_location = -1;
        last_action = 0;
        pre_token = 0;
        current_token = 0;
        current_state = 0;
    }


    inline bool check_token(int time, std::vector<int> &g, int action, int &this_token) {
        // 1: jump, 2: pass, 3: read
        int idxx = std::ceil((double)time / 1800);
        if (action == JUMP) {
            // 表示当前操作是jump
            this_token = G + g[idxx];
        }
        if (action == PASS) {
            // 表示当前操作是pass
            this_token = 1;
        }
        if (action == READ) {
            // 表示当前操作会是read
            if (last_action == JUMP || last_action == PASS) {
                // 如果上一个操作是jump或者pass或者说这一时间片第一次读
                this_token = 64;
            }else {
                this_token = std::max(16.0, std::ceil(pre_token * 0.8));
            }                    
        }
        if (current_token >= this_token) {            
            return true;
        }
        return false;
    }

    inline void update_token(int time, std::vector<int> &g, int action) {
        int result = 0;
        if (check_token(time, g, action, result)) {
            current_token -= result;
            if (action == READ) {
                pre_token = result;
            }
            last_action = action;
        }
    }
};

#endif // !POINT_HPP