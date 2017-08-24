//
// ping.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//#define BOOST_ASIO_ENABLE_HANDLER_TRACKING 1
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <streambuf>
#include <istream>
#include <iostream>
#include <ostream>
#include <unordered_map>
#include <memory>

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

#include "bridge.hpp"

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;
namespace posix_time = boost::posix_time;

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

class Socket
{
    public:
    static Socket& Instance();
    static boost::asio::io_service& GetIOService();
    static icmp::socket& GetSocket();
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    bool add_data(const std::string& ip, int timeout, int repeat);
    void delete_data(const std::string& ip);
    static void start_send(Data* ptr, const boost::system::error_code& error);
    static void handle_timeout(Data* ptr, const boost::system::error_code& error);
    void destroy(const std::string& ip);

    void check_data(const std::string& ip);
private:

    void start_receive();
    void handle_receive(std::size_t length);
    Socket(): m_socket(m_ioService, icmp::v4()) { start_receive(); }

    boost::asio::io_service m_ioService;
    icmp::socket m_socket;
    boost::asio::streambuf reply_buffer_;
public:
    std::unordered_map<std::string, Data> m_data;

};

Data::Data(boost::asio::io_service& ioService, const std::string& dest, 
   int timeout, int repeat)
   : timer_(ioService), m_count(count++),
     m_timeout(timeout), m_repeat(repeat),
     m_destroy(false), m_send(false)
{
    std::cout << "created data: " << m_count << " ptr: " << this << std::endl;
    icmp::resolver::query query(icmp::v4(), dest, "");
    icmp::resolver resolver_(ioService);
    destination_ = *resolver_.resolve(query);
    boost::system::error_code error;
    Socket::start_send(this, error);

}

    Socket& Socket::Instance()
    {
        static Socket socket;
        return socket;
    }

    boost::asio::io_service& Socket::GetIOService() 
    {
        return Instance().m_ioService;
    }


    icmp::socket& Socket::GetSocket() 
    {
        return Instance().m_socket;
    }


bool Socket::add_data(const std::string& ip, int timeout, int repeat)
{
    std::cout << "add: " << ip << std::endl;
    if (m_data.find(ip) != m_data.end())
    {
        std::cout << "already exist" << std::endl;
        return false;
    }
    std::cout << "emplace" << std::endl;
    if (m_data.empty())
    {
        start_receive();
    }

    auto it = m_data.emplace(std::piecewise_construct,
                   std::forward_as_tuple(ip),
                    std::forward_as_tuple(m_ioService, ip, timeout, repeat));
    std::cout << &(it.first->second) << std::endl;
    return true;
}

void Socket::delete_data(const std::string& ip)
{
    std::cout << "delete: " << ip << std::endl;
    auto it = m_data.find(ip);
    if (it == m_data.end())
    {
        std::cout << "not exist" << std::endl;
        return;
    }

    it->second.destroy();
}

void Socket::check_data(const std::string& ip)
{
    std::cout << "check_data: " << ip << std::endl;
    auto it = m_data.find(ip);
    if (it == m_data.end())
    {
        std::cout << "not exist" << std::endl;
        return;
    }

    if (!it->second.m_send)
    {
        std::cout << "We not send anything" << std::endl;
    }

    it->second.m_send = false;
    it->second.timer_.cancel();
}

void Socket::destroy(const std::string& ip)
{
    std::cout << "destroy" << std::endl;
    m_data.erase(ip);
}

  void Socket::start_send(Data* ptr, const boost::system::error_code& error)
  {
    std::cout << "start_send: " << ptr->m_count << std::endl;
    if (error)
    {
        std::cout << "error in start_send: " << error.message() << std::endl;
        if (ptr->m_destroy)
        {
            std::cout << "Destroy" << std::endl;
            Socket::Instance().destroy(ptr->destination_.address().to_string());
            return;
        }
      return;
    }
    
    std::string body("\"Hello!\" from Asio ping.");

    // Create an ICMP header for an echo request.
    icmp_header echo_request;
    echo_request.type(icmp_header::echo_request);
    echo_request.code(0);
    //echo_request.identifier(get_identifier());
    //echo_request.sequence_number(m_count);
    compute_checksum(echo_request, body.begin(), body.end());

    // Encode the request packet.
    boost::asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << echo_request << body;

    // Send the request.
    //time_sent_ = posix_time::microsec_clock::universal_time();
    Socket::Instance().GetSocket().send_to(request_buffer.data(), ptr->destination_);
    // Wait up to five seconds for a reply.
    //timer_.expires_at(time_sent_ + posix_time::seconds(m_timeout));
    ptr->m_send = true;
    ptr->timer_.expires_from_now(posix_time::seconds(ptr->m_timeout));
    ptr->timer_.async_wait(boost::bind(&Socket::handle_timeout, ptr, _1));
  }

  void Socket::handle_timeout(Data* ptr, const boost::system::error_code& error)
  {
    ptr->m_send = false;
    std::cout << "m_count= " << ptr->m_count << std::endl;
    if (!error)
    {
      std::cout << "Request timed out" << std::endl;
    }
    else
    {
        if (ptr->m_destroy)
        {
            std::cout << "Destroy" << std::endl;
            Socket::Instance().destroy(ptr->destination_.address().to_string());
            return;
        }
        std::cout << ptr->m_count << " Error: " << error.message() << std::endl;
      std::cout << "Good ping wait" << std::endl;
    }
    // Requests must be sent no less than one second apart.
    //ptr->timer_.expires_at(time_sent_ + posix_time::seconds(m_repeat));
    ptr->timer_.expires_from_now(posix_time::seconds(ptr->m_repeat));
    ptr->timer_.async_wait(boost::bind(&Socket::start_send, ptr, _1));
  }
  void Socket::start_receive()
  {
    // Discard any data already in the buffer.
    reply_buffer_.consume(reply_buffer_.size());

    // Wait for a reply. We prepare the buffer to receive up to 64KB.
    m_socket.async_receive(reply_buffer_.prepare(65536),
        boost::bind(&Socket::handle_receive, this, _2));
  }

  void Socket::handle_receive(std::size_t length)
  {
    // The actual number of bytes received is committed to the buffer so that we
    // can extract it using a std::istream object.
    reply_buffer_.commit(length);
    std::cout << "length: " << length << std::endl;

    // Decode the reply packet.
    std::istream is(&reply_buffer_);
    ipv4_header ipv4_hdr;
    icmp_header icmp_hdr;
    is >> ipv4_hdr >> icmp_hdr;

    // We can receive all ICMP packets received by the host, so we need to
    // filter out only the echo replies that match the our identifier and
    // expected sequence number.
    std::cout << static_cast<unsigned>(icmp_hdr.type()) << std::endl;
    if (is && icmp_hdr.type() == icmp_header::echo_reply)
    {

      // Print out some information about the reply packet.
      posix_time::ptime now = posix_time::microsec_clock::universal_time();
      std::cout 
        << length - ipv4_hdr.header_length()
        << " bytes from " << ipv4_hdr.source_address()
        << ": icmp_seq=" << icmp_hdr.sequence_number()
        << ", ttl=" << ipv4_hdr.time_to_live()
        << std::endl;
        check_data(ipv4_hdr.source_address().to_string());
    }
    start_receive();
    
  }

int main(int argc, char* argv[])
{
    Socket& p = Socket::Instance(); 
    std::string r1("172.16.5.41");
    std::string r2("172.16.5.52");
    std::string r3("172.16.5.43");
    p.add_data(r1, 3, 3);
    p.add_data(r2, 5, 6);
    p.add_data(r3, 5, 12);

    //deadline_timer timer_(p.GetIOService());
    //timer_.expires_from_now(boost::posix_time::seconds(1));
    //timer_.async_wait([&r3, &timer_, &p](const boost::system::error_code&){ std::cout << "delete" << std::endl; p.delete_data(p.m_data.begin()->first); p.add_data(r3, 3, 3);});

    p.GetIOService().run();
}
/*
using namespace boost::asio;
io_service service;
void func(int i) 
{
    std::cout << "func called, i= " << i << std::endl;
}
void run_dispatch_and_post() 
{
    for ( int i = 0; i < 10; i += 2) 
    {
        service.dispatch(boost::bind(func, i));
        std::cout << "1" << std::endl;
        service.post(boost::bind(func, i + 1));
    }
}
int main(int argc, char* argv[])
 {
    service.post(run_dispatch_and_post);
    std::cout << "1" << std::endl;
    service.run();
}*/