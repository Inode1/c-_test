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

#include "implement.hpp"
#include <string>
int main(int argc, char* argv[])
{
    //Socket& p = Socket::Instance(); 
    Task p;
    std::string r1("172.16.5.41");
    std::string r2("172.16.5.52");
    std::string r3("192.168.122.1");
    p.addRecord(r1, 3, 3);
    p.addRecord(r2, 5, 6);
    p.addRecord(r3, 5, 12);

    deadline_timer timer_(p.m_ioService);
    timer_.expires_from_now(boost::posix_time::seconds(11));
    timer_.async_wait([&p, &timer_, &r1](const boost::system::error_code&){ 
        std::cout << "delete" << std::endl; 
        p.deleteRecord(r1); 
        p.addRecord(r1, 3, 3);
        timer_.expires_from_now(boost::posix_time::seconds(11));
        
    });


    p.Run();
    
    //p.GetIOService().run();
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