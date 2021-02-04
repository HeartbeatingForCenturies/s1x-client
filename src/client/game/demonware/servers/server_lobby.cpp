#include <std_include.hpp>
#include "../demonware.hpp"

namespace demonware
{

	server_lobby::server_lobby(std::string name) : server_base(std::move(name))
	{
		this->register_service<bdAnticheat>();
		this->register_service<bdBandwidthTest>();
		this->register_service<bdContentStreaming>();
		this->register_service<bdDML>();
		this->register_service<bdEventLog>();
		this->register_service<bdGroups>();
		this->register_service<bdStats>();
		this->register_service<bdStorage>();
		this->register_service<bdTitleUtilities>();
		this->register_service<bdProfiles>();
		this->register_service<bdRichPresence>();
		this->register_service<bdFacebook>();
		this->register_service<bdUNK63>();
		this->register_service<bdUNK80>();
		this->register_service<bdPresence>();
		this->register_service<bdUNK104>();
		this->register_service<bdMatchMaking2>();
		this->register_service<bdMarketing>();
	};

	void server_lobby::frame()
	{
		if (!this->incoming_queue_.empty())
		{
			std::lock_guard _(this->mutex_);
			const auto packet = this->incoming_queue_.front();
			this->incoming_queue_.pop();

			this->dispatch(packet);
		}
	}

	int server_lobby::recv(const char* buf, int len)
	{
		if (len <= 3) return -1;
		std::lock_guard<std::recursive_mutex> _(this->mutex_);

		this->incoming_queue_.push(std::string(buf, len));

		return len;
	}

	int server_lobby::send(char* buf, int len)
	{
		if (len > 0 && !this->outgoing_queue_.empty())
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex_);

			len = std::min(len, static_cast<int>(this->outgoing_queue_.size()));
			for (auto i = 0; i < len; ++i)
			{
				buf[i] = this->outgoing_queue_.front();
				this->outgoing_queue_.pop();
			}

			return len;
		}

		return SOCKET_ERROR;
	}

	bool server_lobby::pending_data()
	{
		std::lock_guard _(this->mutex_);
		return !this->outgoing_queue_.empty();
	}

	void server_lobby::send_reply(reply* data)
	{
		if (!data) return;

		std::lock_guard _(this->mutex_);

		const auto buffer = data->data();
		for (auto& byte : buffer)
		{
			this->outgoing_queue_.push(byte);
		}
	}

	void server_lobby::dispatch(const std::string& packet)
	{
		byte_buffer buffer(packet);
		buffer.set_use_data_types(false);

		try
		{
			while (buffer.has_more_data())
			{
				int size;
				buffer.read_int32(&size);

				if (size <= 0)
				{
					const std::string zero("\x00\x00\x00\x00", 4);
					raw_reply reply(zero);
					this->send_reply(&reply);
					return;
				}
				else if (size == 0xC8)
				{
#ifdef DEBUG
					printf("[demonware]: [lobby]: received client_header_ack.\n");
#endif

					int c8;
					buffer.read_int32(&c8);
					std::string packet_1 = buffer.get_remaining();
					demonware::queue_packet_to_hash(packet_1);

					const std::string packet_2("\x16\x00\x00\x00\xab\x81\xd2\x00\x00\x00\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37", 26);
					demonware::queue_packet_to_hash(packet_2);

					raw_reply reply(packet_2);
					this->send_reply(&reply);
#ifdef DEBUG
					printf("[demonware]: [lobby]: sending server_header_ack.\n");
#endif
					return;
				}

				if (buffer.size() < size_t(size)) return;

				uint8_t check_AB;
				buffer.read_byte(&check_AB);
				if (check_AB == 0xAB)
				{
					uint8_t type;
					buffer.read_byte(&type);

					if (type == 0x82)
					{
#ifdef DEBUG
						printf("[demonware]: [lobby]: received client_auth.\n");
#endif
						std::string packet_3(packet.data(), packet.size() - 8); // this 8 are client hash check?

						demonware::queue_packet_to_hash(packet_3);
						demonware::derive_keys_s1();

						char buff[14] = "\x0A\x00\x00\x00\xAB\x83";
						std::memcpy(&buff[6], demonware::get_response_id().data(), 8);
						std::string response(buff, 14);

						raw_reply reply(response);
						this->send_reply(&reply);

#ifdef DEBUG
						printf("[demonware]: [lobby]: sending server_auth_done.\n");
#endif
						return;
					}
					else if (type == 0x85)
					{
						uint32_t msgCount;
						buffer.read_uint32(&msgCount);

						char seed[16];
						buffer.read(16, &seed);

						std::string enc = buffer.get_remaining();

						char hash[8];
						std::memcpy(hash, &(enc.data()[enc.size() - 8]), 8);

						std::string dec = utils::cryptography::aes::decrypt(std::string(enc.data(), enc.size() - 8), std::string(seed, 16), demonware::get_decrypt_key());

						byte_buffer serv(dec);
						serv.set_use_data_types(false);

						uint32_t serv_size;
						serv.read_uint32(&serv_size);

						uint8_t magic; // 0x86
						serv.read_byte(&magic);

						uint8_t service_id;
						serv.read_byte(&service_id);

						this->call_service(service_id, serv.get_remaining());

						return;
					}
				}

				printf("[demonware]: [lobby]: ERROR! received unk message.\n");
				return;
			}
		}
		catch (...)
		{
		}
	}

	void server_lobby::call_service(std::uint8_t id, const std::string& data)
	{
		const auto& it = this->services_.find(id);

		if (it != this->services_.end())
		{
			it->second->exec_task(this, data);
		}
		else
		{
			printf("[demonware]: [lobby]: missing service '%s'\n", utils::string::va("%d", id));

			// return no error
			byte_buffer buffer(data);
			std::uint8_t task_id;
			buffer.read_byte(&task_id);

			this->create_reply(task_id)->send();
		}
	}

}
