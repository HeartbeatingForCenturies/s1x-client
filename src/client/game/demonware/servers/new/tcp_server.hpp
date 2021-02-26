#pragma once

#include "base_server.hpp"

#include <queue>
#include <utils/concurrency.hpp>

class tcp_server : public base_server
{
public:
	using base_server::base_server;

	void handle_input(const char* buf, size_t size)
	{
		in_queue_.access([&](data_queue& queue)
		{
			queue.emplace(buf, size);
		});
	}

	size_t handle_output(char* buf, size_t size)
	{
		if (out_queue_.get_raw().empty())
		{
			return 0;
		}

		out_queue_.access([&](stream_queue& queue)
		{
			for (size_t i = 0; i < size; ++i)
			{
				if (queue.empty())
				{
					return i;
				}

				buf[i] = queue.front();
				queue.pop();
			}

			return size;
		});

		return 0;
	}

protected:
	virtual void handle(const std::string& data) = 0;

	void send(const std::string& data)
	{
		out_queue_.access([&](stream_queue& queue)
		{
			for (const auto& val : data)
			{
				queue.push(val);
			}
		});
	}

private:
	using stream_queue = std::queue<char>;
	using data_queue = std::queue<std::string>;

	utils::concurrency::container<data_queue> in_queue_;
	utils::concurrency::container<stream_queue> out_queue_;

	void frame() override
	{
		if (this->in_queue_.get_raw().empty())
		{
			return;
		}

		while (true)
		{
			std::string packet{};
			const auto result = this->in_queue_.access<bool>([&](data_queue& queue)
			{
				if (queue.empty())
				{
					return false;
				}

				packet = std::move(queue.front());
				queue.pop();
				return true;
			});

			if (!result)
			{
				break;
			}

			this->handle(packet);
		}
	}
};
