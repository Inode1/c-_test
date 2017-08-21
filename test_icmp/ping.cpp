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

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;
namespace posix_time = boost::posix_time;

class pinger;



void check_data(const std::string& ip);

class Socket
{
public:
    static Socket& Instance()
    {
        static Socket socket;
        return socket;
    }

    static icmp::socket& GetSocket() 
    {
        return Instance().m_socket;
    }

    static boost::asio::io_service& GetIOService() 
    {
        return Instance().m_ioService;
    }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
private:
  void start_receive()
  {
    // Discard any data already in the buffer.
    reply_buffer_.consume(reply_buffer_.size());

    // Wait for a reply. We prepare the buffer to receive up to 64KB.
    m_socket.async_receive(reply_buffer_.prepare(65536),
        boost::bind(&Socket::handle_receive, this, _2));
  }

  void handle_receive(std::size_t length)
  {
    // The actual number of bytes received is committed to the buffer so that we
    // can extract it using a std::istream object.
    reply_buffer_.commit(length);

    // Decode the reply packet.
    std::istream is(&reply_buffer_);
    ipv4_header ipv4_hdr;
    icmp_header icmp_hdr;
    is >> ipv4_hdr >> icmp_hdr;

    // We can receive all ICMP packets received by the host, so we need to
    // filter out only the echo replies that match the our identifier and
    // expected sequence number.
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
        m_ioService.dispatch(std::bind(check_data, ipv4_hdr.source_address().to_string()));
    }
    start_receive();
    
  }

    Socket(): m_socket(m_ioService, icmp::v4()) { start_receive(); }
    boost::asio::io_service m_ioService;
    icmp::socket m_socket;
    boost::asio::streambuf reply_buffer_;

};


class pinger
{
public:
  pinger(const pinger&) = delete;
  pinger& operator=(const pinger&) = delete;
  

  pinger(const char* destination, int timeout, int repeat)
    : resolver_(Socket::Instance().GetIOService()),
      timer_(Socket::Instance().GetIOService()),
      m_timeout(timeout), m_repeat(repeat), 
      sequence_number_(0), num_replies_(0), m_count(count++), ping(true)
  {
    icmp::resolver::query query(icmp::v4(), destination, "");
    destination_ = *resolver_.resolve(query);
    start_send();
  }
  void check_data(const std::string& ip)
  {
    if (!ping)
    {
      return;
    }
    else
    {
      std::cout << m_count << " " << ip << std::endl;
      timer_.cancel();
    }

  }
  ~pinger()
  {
    std::cout << m_count << " is deleted" << std::endl;
  }
private:


  void start_send()
  {
    ping = true;
    std::string body("\"Hello!\" from Asio ping.");

    // Create an ICMP header for an echo request.
    icmp_header echo_request;
    echo_request.type(icmp_header::echo_request);
    echo_request.code(0);
    echo_request.identifier(get_identifier());
    echo_request.sequence_number(m_count);
    compute_checksum(echo_request, body.begin(), body.end());

    // Encode the request packet.
    boost::asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << echo_request << body;

    // Send the request.
    time_sent_ = posix_time::microsec_clock::universal_time();
    Socket::Instance().GetSocket().send_to(request_buffer.data(), destination_);
    // Wait up to five seconds for a reply.
    num_replies_ = 0;
    timer_.expires_at(time_sent_ + posix_time::seconds(m_timeout));
    timer_.async_wait(boost::bind(&pinger::handle_timeout, this, _1));
  }

  void handle_timeout(const boost::system::error_code& error)
  {
    std::cout << "m_count= " << m_count << std::endl;
    if (!error)
    {
      std::cout << "Request timed out" << std::endl;
    }
    else
      std::cout << "Good ping wait" << std::endl;
    ping = false;
    // Requests must be sent no less than one second apart.
    timer_.expires_at(time_sent_ + posix_time::seconds(m_repeat));
    timer_.async_wait(boost::bind(&pinger::start_send, this));
  }

  static unsigned short get_identifier()
  {
#if defined(BOOST_ASIO_WINDOWS)
    return static_cast<unsigned short>(::GetCurrentProcessId());
#else
    return static_cast<unsigned short>(::getpid());
#endif
  }

  icmp::resolver resolver_;
  icmp::endpoint destination_;
  deadline_timer timer_;
  unsigned short sequence_number_;
  posix_time::ptime time_sent_;
  std::size_t num_replies_;
  static int count;
  const int m_count;
  int m_timeout;
  int m_repeat;
  bool ping;
};


std::unordered_map<std::string, std::unique_ptr<pinger>> data;
void check_data(const std::string& ip)
{
    if (data.find(ip) != data.end())
    {
        data[ip]->check_data(ip);
    }
}


int pinger::count = 0;

int main(int argc, char* argv[])
{
  try
  {
    std::string r1("10.10.36.38");
    std::string r2("10.10.36.39");
    std::string r3("172.16.5.43");

    data.insert(std::make_pair(r1, std::unique_ptr<pinger>(new pinger(r1.c_str(), 5, 3))));
    data.insert(std::make_pair(r2, std::unique_ptr<pinger>(new pinger(r2.c_str(), 5, 8))));
    data.insert(std::make_pair(r3, std::unique_ptr<pinger>(new pinger(r3.c_str(), 5, 16))));

  deadline_timer timer_(Socket::GetIOService());

  timer_.expires_from_now(posix_time::seconds(15));
  timer_.async_wait([](const boost::system::error_code&){ std::cout << "deleted " << data.begin()->first << std::endl; data.erase(data.begin());});

  Socket::GetIOService().run();

  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}