#include <iostream>
#include <tuple>
#include <utility>
#include <typeindex>

#include <string>
#include <boost/type_index.hpp>

namespace test
{
    template <typename T, T... >
    struct test_value{};

    template <typename V, typename L>
    struct make_temp;

    
    template <template <typename...> class F, typename... Arg, typename L>
    struct make_temp<F<Arg...>, L>
    {
        template <typename ...> using Fv = L;
        using type = F<Fv<Arg>...>;
        
    };
    
    template <typename V, typename L>
    using type = typename make_temp<V, L>::type;
    
    template <template <typename...> class F, typename... T>
    struct make_transform;
    
    template <template <typename...> class F, 
              template <typename...> class A, typename... Arg
             >
    struct make_transform<F, A<Arg...>>
    {
        using type = A<F<Arg>...>;
    };
    
   template <template <typename...> class F, 
             template <typename...> class A, typename... Arg,
             template <typename...> class B, typename... Brg
             >
    struct make_transform<F, A<Arg...>, B<Brg...>>
    {
        using type = A<F<Arg, Brg>...>;
    };
    
    
    template <template <typename...> class F, typename... T>
    using make_transform_t = typename make_transform<F, T...>::type;

    template <typename...T>
    struct list{};

}



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
        enum {length_type = sizeof...(Arg)};
    };

    template <typename T, T...>
    struct index_sequance{};

    template <typename T, T a, typename I>
    struct make_next_index_sequance;
        
    template <typename T, T a, template <typename C, C...> class N, T... Arg>
    struct make_next_index_sequance<T, a, N<T, Arg...>>
    {
        using type = N<T, a, Arg...>;
    };
        

    // index iplementation
    template <typename T, T N, T M>
    struct next_index_sequance
    {
        using type = typename make_next_index_sequance<T, N, typename next_index_sequance<T, N+1, M>::type>::type;
    };

    template <typename T, T N>
    struct next_index_sequance<T, N, N>
    {
	using type = index_sequance<T, N>;
    };

    template <typename T>
    using make_index_sequance = typename next_index_sequance<unsigned, 0, length<T>::length_type - 1>::type;

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
    
    template <typename T, T Int>
    struct const_index
    {
        enum {value = Int};
    };

    template <typename T>
    struct make_index_sequence_list_imp;
    
    template <template <typename T, T...> class T, typename V, V... Int>
    struct make_index_sequence_list_imp<T<V, Int...>>
    {
        using type = test::list<const_index<V, Int>...>;
    };
    
    template <typename T>
    using make_index_sequence_list = typename make_index_sequence_list_imp<T>::type;

    template <typename C, typename... T>
    using concat_t = typename concat_imp<C, T...>::type;
    
    template <typename C, typename T>
    struct concat_inner_imp;

    template <typename C, template <typename...> class T, typename... Arg>
    struct concat_inner_imp<C, T<Arg...>>
    {
        using type = concat_t<C, Arg...>;
    };
    
    template <typename C, typename T>
    using concat_inner_list_t = typename concat_inner_imp<C, T>::type;

    template <class I>
    struct make_external_imp
    {
	template <typename ...>
        using type = I;
    };

    template <typename V, typename I>
    using make_exernal = test::make_transform_t<make_external_imp<I>::template type, V>;    
    
    template <typename T>
    using make_internal = make_index_sequence_list<make_index_sequance<T>>;
    
    template <typename Res, class... TI, class... TE, typename T>
    Res tuple_cat_(test::list<TI...>&& internal, test::list<TE...>&& external, T&& p)
    {
        return Res{std::get<TI::value>(std::get<TE::value>(p))...};
    }
    
    
    template <typename... T, typename Res = concat_t<std::tuple<>, typename remove_ref<T>::type...>>
    Res tuple_cat(T&&... p)
    {
        using arguments_type = test::list<typename remove_ref<T>::type...>;
    	using indexs = make_index_sequence_list<make_index_sequance<arguments_type>>;
	
	   // transform from [[,,][][,,,,][,]] to [0,0,0, 2,2,2,2,2, 3,3]
	   using external_index = concat_inner_list_t<test::list<>, test::make_transform_t<make_exernal, arguments_type, indexs>>;
    	
    	using internal_index = concat_inner_list_t<test::list<>, test::make_transform_t<make_internal, arguments_type>>;
    		
        return tuple_cat_<Res>(internal_index(), external_index(), std::tuple<T...>(std::forward<T>(p)...));
    }
    

    
    template <int First, int Second, typename... C>
    struct print_tuple_
    {
        void operator()(const std::tuple<C...>& p) const
        {
            std::cout << std::get<First>(p) << "   " << std::endl;
            print_tuple_<First + 1, Second, C...>()(p);
        }
    };

    template <int First, typename...C>
    struct print_tuple_<First, First, C...>
    {
        void operator()(const std::tuple<C...>& p) const
        {
        }
    };    

        
    template <typename... T>
    void print_tuple(const std::tuple<T...>& p)
    {
        print_tuple_<0, sizeof...(T), T...>()(p);
    }
}

int main()
{
    std::tuple<int, char, int> a;
    std::tuple<char, int, long long, std::string> b{1, 2, 3, "hello"};
    auto result = naive::tuple_cat(a, std::make_pair<int, double>(1, 2), b, std::tuple<double, char>());
    
    
    naive::print_tuple(result);
    
    
    using indexs = naive::make_index_sequance<decltype(result)>;
    static_assert(naive::length<decltype(result)>::length_type == 11, "not equals 11");
    
    
    
    
    
    
    std::cout << "Len   : " << naive::length<decltype(result)>::length_type << std::endl;
    std::cout << "Indexs: " << boost::typeindex::type_id<indexs>().pretty_name() << std::endl;
    std::cout << "Type  : " << boost::typeindex::type_id<decltype(result)>().pretty_name() << std::endl;
    
    using what = test::test_value<size_t, 2,4,5,6,7,8,9>;
    std::cout << "Value : " <<  boost::typeindex::type_id<what>().pretty_name() << std::endl;
}
