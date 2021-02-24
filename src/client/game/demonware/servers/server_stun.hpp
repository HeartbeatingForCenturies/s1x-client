#pragma once

namespace demonware
{
	class server_stun : public server_base
	{
	public:
		using server_base::server_base;

		int recv(const char* buf, int len) override
		{
			if (len <= 0) return -1;
			std::lock_guard<std::recursive_mutex> _(this->mutex_);
			this->incoming_queue_.push(std::string(buf, len));

			return len;
		}

		int send(char* buf, int len) override
		{
			if (len > 0 && !this->outgoing_queue_.empty())
			{
				std::lock_guard<std::recursive_mutex> _(this->mutex_);

				auto& data = this->outgoing_queue_.front();
				len = std::min(len, static_cast<int>(data.size()));
				for (auto i = 0; i < len; ++i)
				{
					buf[i] = data[i];
				}

				this->outgoing_queue_.pop();

				return len;
			}

			return SOCKET_ERROR;
		}

		bool pending_data() override
		{
			return !outgoing_queue_.empty();
		}

		void frame() override
		{
			if (!this->incoming_queue_.empty())
			{
				std::lock_guard _(this->mutex_);
				const auto packet = this->incoming_queue_.front();
				this->incoming_queue_.pop();

				this->dispatch(packet);
			}
		}

	private:
		std::recursive_mutex mutex_;
		std::queue<std::string> outgoing_queue_;
		std::queue<std::string> incoming_queue_;

		void dispatch(const std::string& packet)
		{
			uint8_t type, version, padding;

			byte_buffer buffer(packet);
			buffer.set_use_data_types(false);
			buffer.read_byte(&type);
			buffer.read_byte(&version);
			buffer.read_byte(&padding);

			switch (type)
			{
			case 30:
				this->ip_discovery();
				break;
			case 20:
				this->nat_discovery();
				break;
			default:
				break;
			}
		}

		void ip_discovery()
		{
			const uint32_t ip = 0x0100007f;

			byte_buffer buffer;
			buffer.set_use_data_types(false);
			buffer.write_byte(31); // type
			buffer.write_byte(2); // version
			buffer.write_byte(0); // version
			buffer.write_uint32(ip); // external ip
			buffer.write_uint16(3074); // port

			send(buffer.get_buffer().data(), static_cast<int>(buffer.get_buffer().size()));
		}

		void nat_discovery()
		{
			const uint32_t ip = 0x0100007f;

			byte_buffer buffer;
			buffer.set_use_data_types(false);
			buffer.write_byte(21); // type
			buffer.write_byte(2); // version
			buffer.write_byte(0); // version
			buffer.write_uint32(ip); // external ip
			buffer.write_uint16(3074); // port
			buffer.write_uint32(this->address()); // server ip
			buffer.write_uint16(3074); // server port

			send(buffer.get_buffer().data(), static_cast<int>(buffer.get_buffer().size()));
		}
	};
}
