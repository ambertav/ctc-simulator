#pragma once

#include <string>
#include <string_view>
#include <fstream>
#include <queue>
#include <mutex>

#include <nlohmann/json.hpp>

#include "constants/constants.h"

struct Entry
{
    std::string system;
    std::string message;
};

class CentralLogger
{
public:
    static CentralLogger &get_instance()
    {
        static CentralLogger instance;
        return instance;
    }

    CentralLogger(const CentralLogger &) = delete;
    CentralLogger &operator=(const CentralLogger &) = delete;
    ~CentralLogger();

    void report(Constants::System system_code, std::string_view message);
    void process();

private:
    std::ofstream outfile;
    std::queue<Entry> message_queue;
    std::mutex mutex;
    nlohmann::json output;

    CentralLogger();

};