CC = g++
LDFLAGS = -lpthread -lboost_system

asio: ping.cpp implement.cpp
	$(CC) -o $@ $^ $(LDFLAGS)