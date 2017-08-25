#include <boost/bind.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include "implement.hpp"
#include "icmp_header.hpp"
#include "ipv4_header.hpp"

using boost::posix_time::ptime;

int Data::count = 0;

bool Task::addRecord(const std::string& ip, unsigned timeout, unsigned repeat)
{
    std::cout << "add: " << ip << std::endl;
    auto range = m_data.equal_range(ip);
    for (auto it = range.first; it != range.second; ++it)
    {
        if (!it->second.m_destroy)
        {
            return false;
        }
    }

    std::cout << "emplace" << std::endl;
    if (m_data.empty())
    {
        startReceive();
    }

    auto it = m_data.emplace(std::piecewise_construct,
                    std::forward_as_tuple(ip),
                    std::forward_as_tuple(m_ioService, ip, timeout, repeat));
    boost::system::error_code error;
    startSend(&it->second, error);
    return true;
}

void Task::deleteRecord(const std::string& ip)
{
    std::cout << "delete: " << ip << std::endl;
    auto range = m_data.equal_range(ip);
    auto it = std::find_if(range.first, range.second, [](const auto& p) 
        {
            return p.second.m_destroy;
        }
    );

    if (it == range.second)
    {
        std::cout << "not exist" << std::endl;
        return;
    }

    it->second.destroy();
}


void Task::changeTimout(const std::string& ip, unsigned timeout)
{

}

void Task::changeRepeat(const std::string& ip, unsigned repeat)
{
}

void Task::checkData(const std::string& ip)
{
    auto it = m_data.find(ip);
    if (it == m_data.end())
    {
        std::cout << "Unknown icmp" << std::endl;
        return;
    }
    auto& data = it->second;
    if (data.m_send)
    {
        std::cout << "icmp from : " << data.m_count << std::endl;
        data.timer_.cancel();
    }
    else
    {
        std::cout << "nobody is now wait for this icmp" << std::endl;
    }

}

void Task::startReceive()
{
    // Discard any data already in the buffer.
    m_replyBuffer.consume(m_replyBuffer.size());

    // Wait for a reply. We prepare the buffer to receive up to 64KB.
    m_socket.async_receive(m_replyBuffer.prepare(65536),
        boost::bind(&Task::handleReceive, this, _1, _2));
}

void Task::handleReceive(const boost::system::error_code& error, std::size_t length)
{
    // The actual number of bytes received is committed to the buffer so that we
    // can extract it using a std::istream object.
    m_replyBuffer.commit(length);

    // Decode the reply packet.
    std::istream is(&m_replyBuffer);
    ipv4_header ipv4_hdr;
    icmp_header icmp_hdr;
    is >> ipv4_hdr >> icmp_hdr;

    // We can receive all ICMP packets received by the host, so we need to
    // filter out only the echo replies that match the our identifier and
    // expected sequence number.
    if (is && icmp_hdr.type() == icmp_header::echo_reply)
    {

      // Print out some information about the reply packet.
      ptime now = boost::posix_time::microsec_clock::universal_time();
      std::cout 
        << length - ipv4_hdr.header_length()
        << " bytes from " << ipv4_hdr.source_address()
        << ": icmp_seq=" << icmp_hdr.sequence_number()
        << ", ttl=" << ipv4_hdr.time_to_live()
        << std::endl;
        checkData(ipv4_hdr.source_address().to_string());
    }
    startReceive();
}

void Task::startSend(Data* ptr, const boost::system::error_code& error)
{
    std::cout << "start_send: " << ptr->m_count << std::endl;
    if (error)
    {
        std::cout << "error in start_send: " << error.message() << std::endl;
        if (ptr->m_destroy)
        {
            std::cout << "Destroy" << std::endl;
            m_data.erase(ptr->destination_.address().to_string());
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
    boost::asio::streambuf requestBuffer;
    std::ostream os(&requestBuffer);
    os << echo_request << body;

    // Send the request.
    //time_sent_ = posix_time::microsec_clock::universal_time();
    m_socket.send_to(requestBuffer.data(), ptr->destination_);
    // Wait up to five seconds for a reply.
    //timer_.expires_at(time_sent_ + posix_time::seconds(m_timeout));
    ptr->m_send = true;
    ptr->timer_.expires_from_now(boost::posix_time::seconds(ptr->m_timeout));
    ptr->timer_.async_wait(boost::bind(&Task::handleTimeout, this, ptr, _1));
}

void Task::handleTimeout(Data* ptr, const boost::system::error_code& error)
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
            m_data.erase(ptr->destination_.address().to_string());
            return;
        }
        std::cout << "Received ping" << std::endl;
    }
    // Requests must be sent no less than one second apart.
    //ptr->timer_.expires_at(time_sent_ + posix_time::seconds(m_repeat));
    ptr->timer_.expires_from_now(boost::posix_time::seconds(ptr->m_repeat));
    ptr->timer_.async_wait(boost::bind(&Task::startSend, this, ptr, _1));
}