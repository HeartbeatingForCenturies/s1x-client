#pragma once
#include <utils/string.hpp>

namespace demonware
{

    class task
    {
    public:
        using callback_t = std::function<void(service_server*, uint8_t, byte_buffer*)>;

    private:
        std::uint8_t id_;
        std::string name_;
        callback_t callback_;

    public:
        virtual ~task() = default;
        task(task&&) = delete;
        task(const task&) = delete;
        task& operator=(const task&) = delete;
        task(std::uint8_t id, std::string name, callback_t callback) : id_(id), name_(std::move(name)), callback_(callback) { }

        auto id() -> std::uint8_t { return this->id_; }
        auto name() -> std::string { return this->name_; }

        void exec(service_server* server, byte_buffer* data)
        {
            this->callback_(server, this->id_, data);
        }
    };

    class service
    {
        std::uint8_t id_;
        std::string name_;
        std::mutex mutex_;
        std::map<std::uint8_t, std::unique_ptr<task>> tasks_;

    public:
        virtual ~service() = default;
        service(service&&) = delete;
        service(const service&) = delete;
        service& operator=(const service&) = delete;
        service(std::uint8_t id, std::string name) : id_(id), name_(std::move(name)) { }

        auto id() -> std::uint8_t { return this->id_; }
        auto name() -> std::string { return this->name_; }

        virtual void exec_task(service_server* server, const std::string& data)
        {
            std::lock_guard $(this->mutex_);

            byte_buffer buffer(data);

            std::uint8_t task_id;
            buffer.read_byte(&task_id);

            const auto& it = this->tasks_.find(task_id);

            if (it != this->tasks_.end())
            {
#ifdef DEBUG
                printf("[demonware]: [%s]: executing task '%s\n", name_.data(), it->second->name().data());
#endif

                it->second->exec(server, &buffer);
            }
            else
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
                printf("[demonware]: [%s]: missing task '%s'\n", name_.data(), utils::string::va("%d", task_id));
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            }
        }

    protected:

        template <typename Class, typename T, typename... Args>
        void register_task(const uint8_t id, std::string name, T(Class::* callback)(Args ...) const)
        {
            this->tasks_[id] = std::make_unique<task>(id, std::move(name), [this, callback](Args ... args) -> T
                {
                    return (reinterpret_cast<Class*>(this)->*callback)(args...);
                });
        }

        template <typename Class, typename T, typename... Args>
        void register_task(const uint8_t id, std::string name, T(Class::* callback)(Args ...))
        {
            this->tasks_[id] = std::make_unique<task>(id, std::move(name), [this, callback](Args ... args) -> T
                {
                    return (reinterpret_cast<Class*>(this)->*callback)(args...);
                });
        }
    };

} // namespace demonware
