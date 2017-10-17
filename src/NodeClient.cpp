//
// Created by dilin on 10/17/17.
//

#include "NodeClient.h"

void NodeClient::read_handler(const boost::system::error_code &ec,
                  std::size_t bytes_transferred)
{
    if (!ec)
    {
        std::cout.write(readBuffer.data(), bytes_transferred);
        tcp_socket.async_read_some(buffer(readBuffer),
                                   bind(&NodeClient::read_handler, shared_from_this(),
                                               placeholders::error,
                                               boost::asio::placeholders::bytes_transferred));
    }
}

void NodeClient::connect_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::string r =
                "Node-" + std::to_string(clientID);
        write(tcp_socket, buffer(r));
        tcp_socket.async_read_some(buffer(readBuffer),
                                   bind(&NodeClient::read_handler, shared_from_this(),
                                        placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
    }
}

void NodeClient::resolve_handler(const boost::system::error_code &ec,
                     tcp::resolver::iterator it)
{
    if (!ec)
        tcp_socket.async_connect(*it,
                                 bind(&NodeClient::connect_handler, shared_from_this(),
                                      placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
}

NodeClient::NodeClient(uint8_t id) :
clientID(id),
tcp_socket(ioservice),
resolv(ioservice)
{
}

NodeClient::NodeClient() :
NodeClient(0)
{
}

void NodeClient::connect(std::string ip, int port)
{
    tcp::resolver::query q(ip,std::to_string(port));
    resolv.async_resolve(q,
                         bind(&NodeClient::resolve_handler, shared_from_this(),
                              placeholders::error,
                              boost::asio::placeholders::bytes_transferred));
    ioservice.run();
}


void NodeClient::getBitBufferFromMat(cv::Mat &binMask,uchar* buffer)
{
    buffer[0] = (uchar)(binMask.rows & 255);
    buffer[1] = (uchar)(binMask.rows  >> 8);
    buffer[2] = (uchar)(binMask.cols & 255);
    buffer[3] = (uchar)(binMask.cols  >> 8);

    int bytePos = 4;
    uchar bitPos = 0;
    uchar temp;
    uchar bit;

    for(int i=0;i<binMask.rows;i++)
    {
        for(int j=0;j<binMask.cols;j++)
        {
            if(binMask.at<uchar>(i,j)>0)
                bit = 1;
            else
                bit = 0;

            if(bitPos==0)
                temp = bit;
            else
                temp = (temp<<1) | bit;

            bitPos++;
            if(bitPos>=8)
            {
                buffer[bytePos] = temp;
                bitPos = 0;
                bytePos++;
            }
        }
    }
}

void NodeClient::publish(cv::Mat binMask)
{
    const int N = (binMask.rows * binMask.cols)/8+5;
    if(bytes== nullptr)
    {
        bytes = new uchar[N];
        byteCount = N;
    }
    assert(byteCount==N);
    getBitBufferFromMat(binMask,bytes);
    async_write(tcp_socket,buffer(bytes,byteCount),
                bind(&NodeClient::write_handler, shared_from_this(),
                     placeholders::error,
                     boost::asio::placeholders::bytes_transferred));
}

void NodeClient::write_handler(const boost::system::error_code &error, std::size_t bytes_transferred)
{
    if(!error)
    {
        std::cout << "Written " << bytes_transferred << " successfully" << std::endl;
    }
}

NodeClient::~NodeClient()
{
    delete [] bytes;
}
