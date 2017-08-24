#ifndef _BRIDGE_HPP_
#define _BRIDGE_HPP_
#include <boost/asio.hpp>
class Bridge
{
public:
    Bridge();
    virtual void addRecord() = 0;
    virtual void deleteRecord() = 0;
    virtual void changeTimout() = 0;
    virtual void changeRepeat() = 0;
    virtual ~Bridge() {};
protected:
    //Interface *GetInterface();
    //View      *GetView();
private:
    //Interface* m_interface;
    //View*      m_view;
};


class SocketIOServiceControl
{
public:
    virtual void stopReceive()                      = 0;
    virtual void startReceive()                     = 0;    
    virtual boost::asio::io_service& GetIOService() = 0;
    virtual boost::asio::ip::icmp::socket& GetSocket() = 0;
    virtual ~SocketIOServiceControl() {};
};

class TaskControl
{
public:
    virtual void CheckData() = 0;
    virtual ~TaskControl() {};
};

#endif