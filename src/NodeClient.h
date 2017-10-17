//
// Created by dilin on 10/17/17.
//

#ifndef NODE_NODECLIENT_HPP
#define NODE_NODECLIENT_HPP

#include<opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <array>
#include <string>
#include <iostream>
#include "MyTypes.h"

using namespace boost::asio;
using namespace boost::asio::ip;


class NodeClient: public boost::enable_shared_from_this<NodeClient>
{
public:
    NodeClient();
    explicit NodeClient(uint8_t id);
    ~NodeClient();
    void connect(std::string ip,int port);
    void publish(cv::Mat binMask);


private:
    io_service ioservice;
    tcp::resolver resolv;
    tcp::socket tcp_socket;
    uchar *bytes = nullptr;
    std::array<char, 1024> readBuffer;
    int byteCount;
    void read_handler(const boost::system::error_code &ec,
                                  std::size_t bytes_transferred);
    void connect_handler(const boost::system::error_code &ec);
    void resolve_handler(const boost::system::error_code &ec,
                         tcp::resolver::iterator it);
    void write_handler(
            const boost::system::error_code& error, // Result of operation.

            std::size_t bytes_transferred           // Number of bytes written from the
            // buffers. If an error occurred,
            // this will be less than the sum
            // of the buffer sizes.
    );
    void getBitBufferFromMat(cv::Mat &binMask, uchar* buffer);
    uint8_t clientID;

};


#endif //NODE_NODECLIENT_HPP
