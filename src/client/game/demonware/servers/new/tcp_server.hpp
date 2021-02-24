#pragma once

#include "base_server.hpp"

#include <queue>
#include <utils/concurrency.hpp>

class tcp_server : public base_server
{
public:
	using base_server::base_server;

	int handle_input(const char* buf, int size)
	{
		return 0;
	}

	int handle_output(char* buf, int size)
	{
		return 0;
	}

protected:
	virtual void handle(const std::string& data) = 0;
	void send(const std::string&)
	{
		
	}

private:
	using stream_queue = std::queue<char>;
	using data_queue = std::queue<std::string>;
	
	utils::concurrency::container<data_queue> in_queue_;
	utils::concurrency::container<stream_queue> out_queue_;

	void frame() override
	{
		
	}
};