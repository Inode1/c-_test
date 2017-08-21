//
// ping.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <streambuf>
#include <istream>
#include <iostream>
#include <ostream>
#include <unordered_map>

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;
namespace posix_time = boost::posix_time;

class pinger;

std::unordered_map<std::string, pinger*> data;

void check_data(const std::string& ip)
{
    if (data.find(ip) != data.end())
    {
        data->check_data()
    }
}

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
    std::cout << "handle_receive: " << length << std::endl;
    
    reply_buffer_.commit(length);

    // Decode the reply packet.
    std::istream is(&reply_buffer_);
    ipv4_header ipv4_hdr;
    icmp_header icmp_hdr;
    is >> ipv4_hdr >> icmp_hdr;

    // We can receive all ICMP packets received by the host, so we need to
    // filter out only the echo replies that match the our identifier and
    // expected sequence number.
    std::cout << "1" << std::endl;
    if (icmp_hdr.type() == icmp_header::echo_request)
    {
        std::cout << "reuqwzt" << std::endl;
    }
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
    std::cout << "2" << std::endl;
    start_receive();
    
  }

    Socket(): m_socket(m_ioService, icmp::v4()) { start_receive(); std::cout << 1 << std::endl;  }
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
      sequence_number_(0), num_replies_(0), m_count(count++)
  {
    icmp::resolver::query query(icmp::v4(), destination, "");
    destination_ = *resolver_.resolve(query);
    std::cout << m_count << std::endl;
    start_send();
  }

private:

  void start_send()
  {
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
    timer_.expires_at(time_sent_ + posix_time::seconds(5));
    timer_.async_wait(boost::bind(&pinger::handle_timeout, this));
  }

  void handle_timeout()
  {
    if (num_replies_ == 0)
      std::cout << "Request timed out" << std::endl;

    // Requests must be sent no less than one second apart.
    timer_.expires_at(time_sent_ + posix_time::seconds(5));
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
};

int pinger::count = 0;




int main(int argc, char* argv[])
{
  try
  {
    std::string r1("172.16.5.41");
    std::string r2("172.16.5.42");
    std::string r3("172.16.5.43");

    pinger a(r1.c_str(), 5, 10);
    data.insert({r1, &a});
    pinger b(r2.c_str(), 5, 10);
    data.insert({r2, &b});
    pinger c(r3.c_str(), 5, 10);
    data.insert({r3, &c});
    Socket::GetIOService().run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}