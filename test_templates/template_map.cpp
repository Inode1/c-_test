#include <iostream>
#include <boost/type_index.hpp>
namespace details
{
    // user type represent integral value
    template <typename Tp, Tp v>
    struct integral_constant
    {
        static constexpr Tp value = v;
    };

    // remove const
    template <typename T>
    struct remove_constant
    {
        using type = T;
    };

    template <typename T>
    struct remove_constant<const T>
    {
        using type = T;
    };

    // user type list of data
    template <typename... L>
    struct list{};    

    // is same type stl template
    template <typename T, typename V>
    struct is_same : integral_constant<bool, false> {};

    template <typename T>
    struct is_same<T, T> : integral_constant<bool, true> {};

    // template plus for list of elements L
    template <typename L>
    struct plus_imp;

    template <typename L>
    using plus = typename plus_imp<L>::type; 

    template <template <typename...> class T, class T1>
    struct plus_imp<T<T1>>
    {
        using type = integral_constant<decltype(T1::value), T1::value>;
    };

    template <template <typename...> class T, typename L1, typename... L>
    struct plus_imp<T<L1, L...>>
    {
        static constexpr auto _v = L1::value + plus<T<L...>>::value;
        using type = integral_constant<typename remove_constant<decltype(_v)>::type, _v>;
    };

    // calculate counts element V in List type L
    template <typename L, typename V>
    struct count_imp;

    template <template <typename...> class T,
              typename... L, typename V
             >
    struct count_imp<T<L...>, V>
    {
        using type = plus<list<is_same<L, V>...>>;
    };

    template <typename L, typename V>
    using count = typename count_imp<L, V>::type;

    // if first then second
    template <typename C, typename F, typename S>
    struct if_then;

    template <typename F, typename S>
    struct if_then<integral_constant<bool, true>, F, S>
    {
        using type = F;
    };

    template <typename F, typename S>
    struct if_then<integral_constant<bool, false>, F, S>
    {
        using type = S;
    };

    // generate list of type
    template <typename B, typename T, int Count>
    struct generate_list_imp;

    template <typename B, typename L, int N>
    using generate_list = typename generate_list_imp<B, L, N>::type;

    // get by one elements
    template <typename T, typename S, int N>
    struct generate_list_element_imp;

    template <template <typename...> class A, typename... Arg,
              template <typename...> class T, typename... L, 
              typename T1, int N
             >
    struct generate_list_element_imp<A<Arg...>, T<T1, L...>, N>
    {
        using type = generate_list<A<Arg..., T1>, T<L...>, N - 1>;
    };


    template <typename B, typename T>
    struct generate_list_imp<B, T, 0>
    {
        using type = B;
    };

    template <template <typename...> class B, typename... Brg,
              template <typename...> class T, typename... Arg,
              int N
             >
    struct generate_list_imp<B<Brg...>, T<Arg...>, N>
    {
        using type = if_then<integral_constant<bool, (N > sizeof...(Arg))>,
                             generate_list<B<Brg..., Arg...>,
                                           T<Arg...>, 
                                           N - sizeof...(Arg)>,
                             generate_list_element_imp<B<Brg...>, 
                                                       T<Arg...>,
                                                       N>::type>::type;
    };
}

namespace details
{
    // set container

}

#define static_assert_integral(typen_name, val)                                \
    using integral_type_##typen_name = details::integral_constant<int, val>;   \
    static_assert(integral_type_##typen_name::value == val, "Fails check user integral type");


int main()
{
    static_assert_integral(five,          5);
    static_assert_integral(six,           6);
    static_assert_integral(seven,         7);
    static_assert_integral(zero,          0);
    static_assert_integral(minus_three,  -3);

    using list_param = details::list<integral_type_five, integral_type_six, 
                                     integral_type_seven, integral_type_zero,
                                     integral_type_minus_three
                                    >;

    using sum = details::plus<list_param>;

    std::cout << boost::typeindex::type_id<sum>().pretty_name() << std::endl;
    //static_assert(sum::value == 15, "Invalid sum");

    static_assert(details::is_same<integral_type_five, integral_type_five>::value == true,
                  "Is same");

    static_assert(details::is_same<integral_type_five, integral_type_six>::value == false,
                  "Is not same");

    using check_if_contain = details::count<list_param, integral_type_five>;
    //std::cout << boost::typeindex::type_id<check_if_contain>().pretty_name() << std::endl;

    using generate_list_100_elements = details::generate_list<details::list, list_param, 100>;
    std::cout << boost::typeindex::type_id<generate_list_100_elements>().pretty_name() << std::endl;    
}