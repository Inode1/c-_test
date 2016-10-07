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

    // get type of list
    template <typename T>
    struct get_typename_og_list;

    template <template <typename ...> class T, typename... L>
    struct get_typename_og_list<T<L...>>
    {
        using type = T<>;
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

    template <template <typename...> class T>
    struct plus_imp<T<>>
    {
        using type = integral_constant<unsigned, 0>;
    };

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
        using type = typename F::type;
    };

    template <typename F, typename S>
    struct if_then<integral_constant<bool, false>, F, S>
    {
        using type = typename S::type;
    };

    // generate list of type
    template <typename B, typename T, unsigned int N>
    struct generate_list_imp;

    template <template <typename...> class B, typename... Brg,
              template <typename...> class T, typename... Arg>
    struct generate_list_imp<B<Brg...>, T<Arg...>, 0>
    {
        using type = B<Brg...>;
    };

    template <typename B, typename L, unsigned int N>
    using generate_list = typename generate_list_imp<B, L, N>::type;

    // get by one elements
    template <typename T, typename S, int N>
    struct generate_list_element_imp;

    template <template <typename...> class A, typename... Arg,
              template <typename...> class T, typename... L, typename T1>
    struct generate_list_element_imp<A<Arg...>, T<T1, L...>, 0>
    {
        using type = A<Arg...>;
    };

    template <template <typename...> class A, typename... Arg,
              template <typename...> class T, typename... L, 
              typename T1, int N
             >
    struct generate_list_element_imp<A<Arg...>, T<T1, L...>, N>
    {
        using type = typename generate_list_element_imp<
                                         A<Arg..., T1>, T<L...>, N - 1
                                         >::type;
    };

    template <template <typename...> class B, typename... Brg,
              template <typename...> class T, typename... Arg,
              unsigned int N
             >
    struct generate_list_imp<B<Brg...>, T<Arg...>, N>
    {
        using is_bool = integral_constant<bool, (N >= sizeof...(Arg))>;
        static constexpr unsigned int value = is_bool::value ? N - sizeof...(Arg) : sizeof...(Arg) - N;
        using type = typename if_then<is_bool,
                             generate_list_imp<B<Brg..., Arg...>,
                                               T<Arg...>, 
                                               value>,
                             generate_list_element_imp<B<Brg...>, 
                                                       T<Arg...>,
                                                       N>>::type;
    };
}

namespace details
{
    // set container
    template <typename R, typename T>
    struct create_set_imp;

    template <typename T>
    using create_set = typename create_set_imp<typename 
                                               get_typename_og_list<T>::type, 
                                               T>::type;

    template <template <typename...> class T,
              typename R
             >
    struct create_set_imp<R, T<>>
    {
        using type = R;
    };

    template <template <typename...> class T, typename... L,
              typename T1, typename... R
             >
    struct create_set_imp<T<R...>, T<T1, L...>>
    {
        using exist = integral_constant<bool, count<T<L...>, T1>::value != 0>; 
        using type  = typename if_then< exist, create_set_imp<T<R...>, T<L...>>,
                                               create_set_imp<T<R..., T1>, T<L...>>
                                      >::type;
    };
}

#define static_assert_integral(typen_name, val)                                \
    using integral_type_##typen_name = details::integral_constant<int, val>;   \
    static_assert(integral_type_##typen_name::value == val, "Fails check user integral type");


int main()
{
    static_assert_integral(one,   1);
    static_assert_integral(two,   2);
    static_assert_integral(three, 3);
    static_assert_integral(thour, 4);
    static_assert_integral(five,  5);

    using list_param = details::list<integral_type_one, integral_type_two, 
                                     integral_type_three, integral_type_thour,
                                     integral_type_five
                                    >;

/*    using sum = details::plus<list_param>;

    std::cout << boost::typeindex::type_id<sum>().pretty_name() << std::endl;
    //static_assert(sum::value == 15, "Invalid sum");

    static_assert(details::is_same<integral_type_one, integral_type_one>::value == true,
                  "Is same");

    static_assert(details::is_same<integral_type_one, integral_type_five>::value == false,
                  "Is not same");

    using check_if_contain = details::count<list_param, integral_type_five>;
    std::cout << boost::typeindex::type_id<check_if_contain>().pretty_name() << std::endl;*/

    using generate_list_100_elements = details::generate_list<details::list<>, list_param, 900>;
    std::cout << boost::typeindex::type_id<generate_list_100_elements>().pretty_name() << std::endl;    

    using check_if_contain_second = details::count<generate_list_100_elements, integral_type_five>;
    std::cout << boost::typeindex::type_id<check_if_contain_second>().pretty_name() << std::endl;    

    //using create_set       = details::create_set<generate_list_100_elements>;
    //std::cout << boost::typeindex::type_id<create_set>().pretty_name() << std::endl;    
    //std::cout << boost::typeindex::type_id<create_set_count>().pretty_name() << std::endl;    
    //std::cout << boost::typeindex::type_id<create_set_count>().pretty_name() << std::endl;    
}