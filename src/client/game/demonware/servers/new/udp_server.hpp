#pragma once

#include "base_server.hpp"

#include <queue>
#include <utils/concurrency.hpp>

class udp_server : public base_server
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

		return out_queue_.access<size_t>([&](data_queue& queue) -> size_t
		{
			if (queue.empty())
			{
				return 0;
			}

			auto data = std::move(queue.front());
			queue.pop();

			const auto copy_size = std::min(size, data.size());
			std::memcpy(buf, data.data(), copy_size);

			return copy_size;
		});
	}

	bool pending_data() override
	{
		return !this->out_queue_.get_raw().empty();
	}

protected:
	virtual void handle(const std::string& data) = 0;

	void send(const std::string& data)
	{
		out_queue_.access([&](data_queue& queue)
		{
			queue.push(data);
		});
	}

private:
	utils::concurrency::container<data_queue> in_queue_;
	utils::concurrency::container<data_queue> out_queue_;

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
