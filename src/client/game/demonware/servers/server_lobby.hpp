#pragma once

namespace demonware
{

	class server_lobby : public server_base, service_server
	{
	public:
		explicit server_lobby(std::string name);

		void frame() override;
		int recv(const char* buf, int len) override;
		int send(char* buf, int len) override;
		bool pending_data()override;

		template <typename T>
		void register_service()
		{
			static_assert(std::is_base_of<service, T>::value, "service must inherit from service");

			auto service = std::make_unique<T>();
			const uint8_t id = service->id();

			this->services_[id] = std::move(service);
		}

		void send_reply(reply* data) override;

	private:
		std::recursive_mutex mutex_;
		std::queue<char> outgoing_queue_;
		std::queue<std::string> incoming_queue_;
		std::map<std::uint8_t, std::unique_ptr<service>> services_;

		void dispatch(const std::string& packet);
		void call_service(std::uint8_t type, const std::string& data);

	};

}