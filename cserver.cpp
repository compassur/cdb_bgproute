#define ASIO_STANDALONE

#include <cstdlib>
#include <iostream>
#include <memory>
#include "asio.hpp"
#include "cdb.h"

using asio::ip::tcp;

cdb<NodeData> route_cdb("ASN", "PREFIX");

class session
    : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket)
        : socket_(std::move(socket)) {
    }

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(asio::buffer(data_, max_length),
            [this, self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    get_cmd(length);
                    parse_cmd();
                    do_write();
                }
            });
    }

    void read_more() {
        auto self(shared_from_this());
        socket_.async_read_some(asio::buffer(data_, max_length),
            [this, self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    get_cmd(length);
                }
            });
    }

    void do_write() {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer(write_data_.c_str(), write_data_.length()),
            [this, self](std::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    do_read();
                }
            });
    }

    void get_cmd(std::size_t length) {
        cmd_s_.append(data_, length);
        if (data_[length-1] != '\n') {
            read_more();
        }
    }

    void parse_cmd() noexcept {
        int order_num = 0;
        std::string cmd_type = "REPLACE";
        std::string tmp;
        NodeData n;

        for (int i = 0; i < cmd_s_.length(); i++) {
            if (cmd_s_[i] == '\t' || cmd_s_[i] == '\n') {
                if (order_num == 0) {
                    cmd_type = tmp;
                } else if (order_num == 1) {
                    if (cmd_type == "SELECT_COL")
                        n.prefix_ = tmp;
                    else
                        n.asn_ = tmp;
                } else if (order_num == 2) {
                    n.prefix_ = tmp;
                } else if (order_num == 3) {
                    n.path_ = tmp;
                } else if (order_num == 4) {
                    n.time_ = std::stoul(tmp);
                }
                order_num++;
                tmp = "";
            } else {
                tmp.push_back(cmd_s_[i]);
            }
        }

        write_data_ = "";
        if (cmd_type == "REPLACE" && order_num == 5) {
            cnode<NodeData> *node = route_cdb.insert_data(n.asn_, n.prefix_, n);
            if (node == nullptr)
                write_data_ = "ERROR\n";
            else {
                write_data_ = "SUCCEED\n";
            }
        } else if (cmd_type == "SELECT" && order_num == 3) {
            cnode<NodeData> *node = route_cdb.get_node(n.asn_, n.prefix_);
            if (node == nullptr)
                write_data_ = "NODATA\n";
            else {
                write_data_ = node->data().to_string();
            }
        } else if (cmd_type == "SELECT_ROW" && order_num == 2) {
            chead<NodeData> *row = route_cdb.get_row(n.asn_);
            if (row == nullptr)
                write_data_ = "NODATA\n";
            else {
                for (cnode<NodeData> *p = row->get_first_node(); p; p=p->rnext()) {
                    write_data_ += p->data().to_string();
                }
            }
        } else if (cmd_type == "SELECT_COL" && order_num == 2) {
            chead<NodeData> *col = route_cdb.get_col(n.prefix_);
            if (col == nullptr)
                write_data_ = "NODATA\n";
            else {
                for (cnode<NodeData> *p = col->get_first_node(); p; p=p->cnext()) {
                    write_data_ += p->data().to_string();
                }
            }
        } else {
            write_data_ = "ERROR\n";
        }

        cmd_s_ = "";
    }

    tcp::socket socket_;
    static const std::size_t max_length= 1024;
    std::string cmd_s_;
    std::string write_data_;
    char data_[max_length];
};

class server {
public:
    server(asio::io_service &io_service, unsigned short port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
          socket_(io_service) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(socket_,
            [this](std::error_code ec) {
                if (!ec) {
                    std::make_shared<session>(std::move(socket_))->start();
                }

                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

int main(int argc, char *argv[]) {
    try {
        asio::io_service io_service;

        server s(io_service, 4444);

        io_service.run();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
