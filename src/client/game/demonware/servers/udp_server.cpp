#include <std_include.hpp>
#include "udp_server.hpp"

namespace demonware
{
	void udp_server::handle_input(const char* buf, size_t size)
	{
		in_queue_.access([&](data_queue& queue)
		{
			queue.emplace(buf, size);
		});
	}

	size_t udp_server::handle_output(char* buf, size_t size)
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

	bool udp_server::pending_data()
	{
		return !this->out_queue_.get_raw().empty();
	}

	void udp_server::send(const std::string& data)
	{
		out_queue_.access([&](data_queue& queue)
		{
			queue.push(data);
		});
	}

	void udp_server::frame()
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
}	