#ifndef _INTERFEACE_HPP
#define _INTERFEACE_HPP

#include <unordered_map>
#include <iostream>

#include "bridge.hpp"

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;
using boost::asio::io_service;
using boost::asio::ip::icmp;

struct Data
{
    Data(boost::asio::io_service& ioService, const std::string& dest, 
       int timeout, int repeat)
       : timer_(ioService), m_count(count++),
         m_timeout(timeout), m_repeat(repeat),
         m_destroy(false), m_send(false),
         destination_(boost::asio::ip::address::from_string(dest), 0)
    {
    }

    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;
    void destroy()
    {
        timer_.cancel();
        m_destroy = true;
    }
    icmp::endpoint destination_;
    deadline_timer timer_;
    unsigned short sequence_number_;
    static int count;
    const int m_count;
    int m_timeout;
    int m_repeat;
    bool m_destroy;
    bool m_send;
};

class Task
{
public:
    Task(): m_socket(m_ioService, icmp::v4()) {}
    // Bridge Interface
    bool addRecord(const std::string& ip, unsigned timeout, unsigned repeat);
    void deleteRecord(const std::string& ip);
    void changeTimout(const std::string& ip, unsigned timeout);
    void changeRepeat(const std::string& ip, unsigned repeat);
    void checkData(const std::string& ip);
    void Run() { m_ioService.run(); }
    io_service m_ioService;
private:
    void startReceive();
    void handleReceive(const boost::system::error_code& error, std::size_t length);

    void startSend(Data* ptr, const boost::system::error_code& error);
    void handleTimeout(Data* ptr, const boost::system::error_code& error);

    icmp::socket m_socket;


    boost::asio::streambuf m_replyBuffer;
    std::unordered_multimap<std::string, Data> m_data;
};
#endif