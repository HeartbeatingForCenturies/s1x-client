#pragma once
#include <utils/cryptography.hpp>

class server_base
{
    std::string name_;
    std::uint32_t address_ = 0;

public:
    server_base(std::string name) : name_(std::move(name))
    {
        this->address_ = utils::cryptography::jenkins_one_at_a_time::compute(this->name_);
    }

    std::uint32_t address()
    {
        return this->address_;
    }

    virtual void frame()
    {
        return;
    }

    virtual int recv(const char* buf, int size)
    {
        printf("PACKET\n%s\n", std::string(buf, size).data());
        return 0;
    }

    virtual int send(char* buf, int size)
    {
        return SOCKET_ERROR;
    }

    virtual bool pending_data()
    {
        return false;
    }
};

using server_ptr = std::shared_ptr<server_base>;

namespace demonware
{
#pragma pack(push, 1)
    struct auth_ticket_t
    {
        unsigned int m_magicNumber;
        char m_type;
        unsigned int m_titleID;
        unsigned int m_timeIssued;
        unsigned int m_timeExpires;
        unsigned __int64 m_licenseID;
        unsigned __int64 m_userID;
        char m_username[64];
        char m_sessionKey[24];
        char m_usingHashMagicNumber[3];
        char m_hash[4];
    };
#pragma pack(pop)

    class server_auth3 : public server_base
    {
    public:
        explicit server_auth3(std::string name) : server_base(name) {};

        void frame() override;
        int recv(const char* buf, int len) override;
        int send(char* buf, int len) override;
        bool pending_data()override;

    private:
        std::recursive_mutex mutex_;
        std::queue<char> outgoing_queue_;
        std::queue<std::string> incoming_queue_;

        void send_reply(reply* data);
        void dispatch(const std::string& packet);
    };

}
