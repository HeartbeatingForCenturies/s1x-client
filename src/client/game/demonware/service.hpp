#pragma once
#include <utils/string.hpp>

namespace demonware
{

class service
{
    using callback_t = std::function<void(service_server*, byte_buffer*)>;

    std::uint8_t id_;
    std::string name_;
    std::mutex mutex_;
    std::uint8_t task_id_;
    std::map<std::uint8_t, callback_t> tasks_;

public:
    virtual ~service() = default;
    service(service&&) = delete;
    service(const service&) = delete;
    service& operator=(const service&) = delete;
    service(std::uint8_t id, std::string name) : id_(id), task_id_(0), name_(std::move(name)) { }

    auto id() const -> std::uint8_t { return this->id_; }
    auto name() const -> std::string { return this->name_; }
    auto task_id() const -> std::uint8_t { return this->task_id_; }

    virtual void exec_task(service_server* server, const std::string& data)
    {
        std::lock_guard $(this->mutex_);

        byte_buffer buffer(data);

        buffer.read_byte(&this->task_id_);

        const auto& it = this->tasks_.find(this->task_id_);

        if (it != this->tasks_.end())
        {
            std::cout << "demonware::" << name_ << ": executing task '" << utils::string::va("%d", this->task_id_) << "'\n";

            it->second(server, &buffer);
        }
        else
        {
            std::cout << "demonware::" << name_ << ": missing task '" << utils::string::va("%d", this->task_id_) << "'\n";

            // return no error
            server->create_reply(this->task_id_)->send();
        }
    }

protected:

    template <typename Class, typename T, typename... Args>
    void register_task(const uint8_t id, T(Class::* callback)(Args ...) const)
    {
        this->tasks_[id] = [this, callback](Args ... args) -> T
        {
            return (reinterpret_cast<Class*>(this)->*callback)(args...);
        };
    }

    template <typename Class, typename T, typename... Args>
    void register_task(const uint8_t id, T(Class::* callback)(Args ...))
    {
        this->tasks_[id] = [this, callback](Args ... args) -> T
        {
            return (reinterpret_cast<Class*>(this)->*callback)(args...);
        };
    }
};

} // namespace demonware
