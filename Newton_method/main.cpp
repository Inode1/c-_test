#include <iostream>
#include <boost/type_index.hpp>
#include <boost/type_index/ctti_type_index.hpp>

#include "ast.h"    

using namespace boost::typeindex;

int main()
{
    terminal<int> a1{2};
    terminal<double> a2{2.001};

    //terminal<int> result;

    foo(1);
    boo<int>(1);

    auto result1 = a1*a1;
    auto result2 = a2*a2;

    std::cout << "Runtime type: " << type_id_runtime(result1).pretty_name() << std::endl
              << " Result type: " << type_id_runtime(result1()).pretty_name() << std::endl 
              << " Result1: " << result1() << " Result2: " << result2() << std::endl
              << " Derivative" << Derivative(result1, result2)() << std::endl; 
}