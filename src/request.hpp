#ifndef REQUEST_HPP
#define REQUEST_HPP


class Request {
    public:
    int request_id; // 请求id
    int object_id;  // 请求的对象id
    int timestamp;  // 请求到来的时间戳
    bool is_done;   // 请求是否完成
    Request(int request_id, int object_id, int timestamp) : request_id(request_id), object_id(object_id), timestamp(timestamp), is_done(false) {

    }

    Request() : request_id(-1), object_id(-1), timestamp(-1), is_done(false) {

    }
};


#endif  // REQUEST_HPP