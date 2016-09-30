#include <iostream>
#include <tuple>
#include <utility>
#include <typeindex>

#include <string>
#include <boost/type_index.hpp>

namespace naive
{
    // local template remove reference
    template <typename T>
    struct remove_ref
    {
        using type = T;
    };

    template <typename T>
    struct remove_ref<T&>
    {
        using type = T;
    };

    // get template param len
    template <typename T>
    struct length;

    template <template <typename...> class T, typename... Arg>
    struct length<T<Arg...>>
    {
        static const unsigned length_type = sizeof...(Arg);
    };

    template <typename... T>
    struct index_sequance{};

    //template <>
    //struct make_next_index_sequance;

    // index iplementation
    template <typename T, T N, T M>
    struct next_index_sequance
    {
        using type = index_sequance<T, typename next_index_sequance<T, N+1, M>::type>;
    };

    template <typename T, T N>
    struct next_index_sequance<T, N, N>
    {
	using type = index_sequance<N>;
    };

    template <typename T>
    using make_index_sequance = typename next_index_sequance<unsigned, 0, typename length<T>::length_type>::type;

    // concat implement
    template <typename... T>
    struct concat_imp;

    template <class T>
    struct concat_imp<T>
    {
        using type = T;
    };
 
    template <
              template <typename...> class F, typename... Fa, 
              template <typename...> class S, typename... Sa,
              typename... All
             >
    struct concat_imp<F<Fa...>, S<Sa...>, All...>
    {
        using temp = F<Fa..., Sa...>;
        using type = typename concat_imp<temp, All...>::type;
    };

    template <typename... T>
    using concat_t = typename concat_imp<std::tuple<>, T...>::type;
    
    template <typename... T, typename Res = concat_t<typename remove_ref<T>::type...>>
    Res tuple_cat(T&&... p)
    {        
        return {};
    }
}

int main()
{
    std::tuple<int, char, int> a;
    std::tuple<char, int, long long, std::string> b;
    auto result = naive::tuple_cat(a, std::make_pair<int, double>(1, 2), b, std::tuple<double, char>());
    using indexs = naive::make_index_sequance<decltype(result)>;
    static_assert(naive::length<decltype(result)>::length_type == 11, "not equals 11");
    //std::cout << "Len   : " << naive::length<decltype(result)>::length_type << std::endl;
    std::cout << "Indexs: " << boost::typeindex::type_id<index>().pretty_name() << std::endl;
    std::cout << "Type  : " << boost::typeindex::type_id<decltype(result)>().pretty_name() << std::endl;
}
