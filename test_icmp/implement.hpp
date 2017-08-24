#ifndef _INTERFEACE_HPP
#define _INTERFEACE_HPP

#include <unordered_map>

#include "bridge.hpp"

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;
using boost::asio::io_service;
using boost::asio::ip::icmp::socket;

class SocketIOService: public SocketIOServiceControl
{
public:
    static SocketIOService* Instance();
    SocketIOService(const SocketIOService&) = delete;
    SocketIOService& operator=(const SocketIOService&) = delete;

    io_service& GetIOService() override;
    socket& GetSocket() override;
    void stopReceive() override;
    void startReceive() override;
private:

    void start_receive();
    void handle_receive(std::size_t length);
    SocketIOService(): m_socket(m_ioService, icmp::v4());

    io_service m_ioService;
    socket m_socket;
    boost::asio::streambuf reply_buffer_;

};

struct Data
{
  Data(boost::asio::io_service& ioService, const std::string& dest, 
       int timeout, int repeat);

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

int Data::count = 0;

class Task: public Bridge, public TaskControl
{
    Task();
    // Bridge Interface
    void addRecord()    override;
    void deleteRecord() override;
    void changeTimout() override;
    void changeRepeat() override;
    // TaskControl Interface
    void CheckData() override;

    static void start_send(Data* ptr, const boost::system::error_code& error);
    static void handle_timeout(Data* ptr, const boost::system::error_code& error);

private:
    SocketIOServiceControl* m_socket;
    std::unordered_map<std::string, Data> m_data;
};
#endif