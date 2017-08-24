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
    virtual void checkData();
}

class SocketIOService
{
public:
    static Socket& Instance();
    boost::asio::io_service& GetIOService();
    icmp::socket& GetSocket();
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    void check_data(const std::string& ip);
private:

    void start_receive();
    void handle_receive(std::size_t length);
    Socket(): m_socket(m_ioService, icmp::v4()) { start_receive(); }

    boost::asio::io_service m_ioService;
    icmp::socket m_socket;
    boost::asio::streambuf reply_buffer_;

};

class Task: public Bridge
{
    Task();
    void addRecord()    override;
    void deleteRecord() override;
    void changeTimout() override;
    void changeRepeat() override;

    static void start_send(Data* ptr, const boost::system::error_code& error);
    static void handle_timeout(Data* ptr, const boost::system::error_code& error);

private:
    Socket* m_socket;
    std::unordered_map<std::string, Data> m_data;
};
