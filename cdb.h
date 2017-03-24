//
// Created by yuans on 2017/3/21.
//

#ifndef CDB_CDB_H
#define CDB_CDB_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <iostream>

#define _ROW_HEAD 0
#define _COL_HEAD 1

#define _NO_REPLACE 0
#define _REPlACE_ALL 1
#define _REPLACE_BY_TIME 2

typedef struct NodeData {
    NodeData() {
        asn_ = "";
        prefix_ = "";
        path_ = "";
        time_ = 0;
    }
    NodeData(const std::string &asn, const std::string &prefix, const std::string &path, const uint32_t &time)
            : asn_(asn), prefix_(prefix), path_(path), time_(time) {}
    NodeData(const NodeData &n) {
        asn_ = n.asn_;
        prefix_ = n.prefix_;
        path_ = n.path_;
        time_ = n.time_;
    }
    NodeData set_data(const std::string &asn, const std::string &prefix, const std::string &path, const uint32_t &time) {
        asn_ = asn;
        prefix_ = prefix;
        path_ = path;
        time_ = time;
        return *this;
    }
    std::string get_rkey() const {
        return asn_;
    }
    std::string get_ckey() const {
        return prefix_;
    }
    void print_data() const {
        std::cout << asn_ << " " << prefix_ << " " << path_ << " " << time_ << std::endl;
    }
    std::string to_string() const {
        return asn_ + "\t" + prefix_ + "\t" + path_ + "\t" + std::to_string(time_) + "\n";
    }
    std::string asn_;
    std::string prefix_;
    std::string path_;
    uint32_t time_;
}NodeData;

template <typename T>
class chead;

template <typename T>
class cnode {
public:
    cnode(const T &d) : data_(d) {
        rnext_ = nullptr;
        cnext_ = nullptr;
    }

    cnode<T> *set_data(const T &d) {
        data_ = d;
        return this;
    }

    uint32_t get_time() {
        return data_.time_;
    }

    cnode<T> *add_row_data(cnode<T> *new_node) {
        rnext_ = new_node;
        return new_node;
    }

    cnode<T> *add_col_data(cnode<T> *new_node) {
        cnext_ = new_node;
        return new_node;
    }

    cnode<T> *rnext() {
        return rnext_;
    }

    cnode<T> *cnext() {
        return cnext_;
    }

    const T& data() {
        return data_;
    }

private:
    T data_;
    cnode<T> *rnext_;
    cnode<T> *cnext_;
};

template <typename T>
class chead {
public:
    chead(const std::string &t, const uint8_t &type) : key_(t), type_(type) {
        first_ = nullptr;
        last_ = nullptr;
    }

    cnode<T> *get_first_node() {
        return first_;
    }

    chead<T> *add_node(cnode<T> *new_node) {
        if (first_ == nullptr) {
            first_ = new_node;
        } else {
            if (type_ == _ROW_HEAD) {
                last_->add_row_data(new_node);
            } else if (type_ == _COL_HEAD) {
                last_->add_col_data(new_node);
            }
        }
        last_ = new_node;
        return this;
    }

private:
    uint8_t type_;
    std::string key_;
    cnode<T> *first_;
    cnode<T> *last_;
};

template<typename T>
class cdb {
public:
    cdb(const std::string &rname, const std::string &cname) : rname_(rname), cname_(cname) {}

    cnode<T> *insert_data(const std::string &rkey, const std::string &ckey,
                          const T &data, int replace=_REPLACE_BY_TIME) {
        auto ntree_iter = ntree_.find(_get_key(rkey, ckey));
        chead<T> *rhead_ptr = nullptr, *chead_ptr = nullptr;
        cnode<T> *node_ptr = nullptr;
        if (ntree_iter != ntree_.end()) {
            if (replace == _REPLACE_BY_TIME) {
                if (data.time_ > ntree_iter->second->get_time()) {
                    return ntree_iter->second->set_data(data);
                }
            } else if (replace == _REPlACE_ALL) {
                return ntree_iter->second->set_data(data);
            }
        } else {
            auto rhead_iter = rtree_.find(rkey);
            if (rhead_iter == rtree_.end()) {
                rhead_ptr = new chead<T>(rkey, _ROW_HEAD);
                rtree_[rkey] = rhead_ptr;
            } else {
                rhead_ptr = rhead_iter->second;
            }
            auto chead_iter = ctree_.find(ckey);
            if (chead_iter == ctree_.end()) {
                chead_ptr = new chead<T>(ckey, _COL_HEAD);
                ctree_[ckey] = chead_ptr;
            } else {
                chead_ptr = chead_iter->second;
            }
            node_ptr = new cnode<T>(data);
            rhead_ptr->add_node(node_ptr);
            chead_ptr->add_node(node_ptr);
            ntree_[_get_key(rkey, ckey)] = node_ptr;
            return node_ptr;
        }
        return nullptr;
    }

    cnode<T> *get_node(const std::string &rkey, const std::string &ckey) {
        auto ntree_iter = ntree_.find(_get_key(rkey, ckey));
        if (ntree_iter != ntree_.end()) {
            return ntree_iter->second;
        }
        return nullptr;
    }

    chead<T> *get_row(const std::string &rkey) {
        auto rhead_iter = rtree_.find(rkey);
        if (rhead_iter != rtree_.end()) {
            return rhead_iter->second;
        }
        return nullptr;
    }

    chead<T> *get_col(const std::string &ckey) {
        auto chead_iter = ctree_.find(ckey);
        if (chead_iter != ctree_.end()) {
            return chead_iter->second;
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, chead<T> *> rtree_;
    std::unordered_map<std::string, chead<T> *> ctree_;
    std::unordered_map<std::string, cnode<T> *> ntree_;
    std::string rname_;
    std::string cname_;

private:
    const std::string _get_key(const std::string &rkey, const std::string &ckey) {
        return rkey + ckey;
    }
};


#endif //CDB_CDB_H
