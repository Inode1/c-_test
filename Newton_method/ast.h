// The Curiously Recurring Template Pattern (CRTP)
template<class Derived>
struct Base
{
    // methods within Base can use template to access members of Derived
    auto operator()() const
    {
        return  static_cast<const Derived &>(*this)();   
    }
};

template<class Tag, class Child>
struct unary_expr: public Base<unary_expr<Tag, Child>>
{
    explicit unary_expr(const Child& p): c(p) {}

    auto operator()() const { return op(c()); }

    private:
    Child c;
    Tag op;
};

template<class Tag, class Child0, class Child1>
struct binary_expr: public Base<binary_expr<Tag, Child0, Child1>>
{
    explicit binary_expr(const Child0& p0, const Child1& p1): c0(p0), c1(p1) {}

    auto operator()() const { return op(c0(), c1());}

    private:
    Child0 c0;
    Child1 c1;
    Tag op;
};

template<class T>
struct terminal: public Base<terminal<T>>
{
    terminal() = default;
    explicit terminal(const T& v): value(v) {}
    auto operator()() const { return value; } 

    template <class X>
    terminal& operator=(const Base<X>& that)
    {
        value = that();
        return *this;
    }

    private:
    T value;
};

// plus 
struct plus_
{   
    template <class T, class U>
    auto operator()(const T& lhs, const U& rhs) const { return lhs + rhs; };
};
    
template <class E1, class E2>
binary_expr<plus_, E1, E2> operator+(const E1& lhs, const E2& rhs)
{
    return binary_expr<plus_, E1, E2>{lhs, rhs};
};

// multiple
struct multiple_
{
    template <class E1, class E2>
    auto operator()(const E1& lhs, const E2& rhs) const 
    {
        return lhs * rhs;
    }
};

template <class E1, class E2>
binary_expr<multiple_, E1, E2> operator*(const E1& lhs, const E2& rhs)
{
    return binary_expr<multiple_, E1, E2>{lhs, rhs};
};

// divide
struct divide_
{
    template <class E0, class E1>
    auto operator()(const E0& lhs, const E1& rhs)
    {
        return lhs / rhs;
    }
};

template <class E1, class E2>
binary_expr<divide_, E1, E2> operator/(const E1& lhs, const E2& rhs)
{
    return binary_expr<multiple_, E1, E2>{lhs, rhs};
};

// unary operator
struct minus_
{
    template <class E>
    auto operator()(const E& lhs) const { return -lhs;}
};


template <class E>
unary_expr<minus_, E> operator-(const E& lhs)
{
    return unary_expr<minus_, E>{lhs};
}

// derivative

struct Derivative
{
    template <class X, class Y>
    Derivative(const Base<X>& function1, const Base<Y>& function2)
    {
        result = (function2() - function1()) / 0.001;
    }

    double operator()()
    {
        return result;
    }

    private:
    double result;
};

template <class T>
typename std::enable_if<std::is_arithmetic<T>::value>::type foo(const T& p) {}

template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, T>::type = 0>
void boo(const T& p) { }
/*template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
void boo(const T& bar) { }*/