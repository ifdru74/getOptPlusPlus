//
// Created by shtykov on 6/26/25.
//

#ifndef Config_H
#define Config_H
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "appArguments.h"

namespace app::config {

    enum class ParseResult {
        None,
        Error,
        HelpRequired,
        Parsed,
        Unknown,
        BadOptionIndex
    };
    //typedef struct option Option;
    class Config {
    public:
        static constexpr const size_t MANDATORY_PASSED_ALL = -1;
        Config(const Config&) = delete; // no copy constructor
        Config& operator=(const Config&) = delete; // no assignment operator
        Config(Config&&) = delete; // no move constructor
        Config& operator=(Config&&) = delete;// no move assignment
        static Config& getInstance() {
            std::call_once(initFlag, []() {
                instance.reset(new Config());
            });
            return *instance;
        }
        ParseResult parseArgs(int argc, char** argv, const std::vector<app::arguments::Options>& optMap);
        [[nodiscard]] app::arguments::Value getValue(const std::string& name) const;
        [[nodiscard]] bool hasValue(const std::string& name) const noexcept;
        static std::ostream& printHelp(const std::vector<app::arguments::Options>& optMap, const std::string& progName, std::ostream& out);
        size_t  valid(const std::vector<app::arguments::Options>& optMap);
        size_t badArgumentIndex;
        size_t badArgumentOffset;
        size_t lastParsed;
    private:
        std::map<std::string,app::arguments::Value> config;
        Config() : badArgumentIndex(0), badArgumentOffset(0), lastParsed(0), config() {};
        static std::unique_ptr<Config> instance;
        static std::once_flag initFlag;
        void fillIn(const app::arguments::Options& opt, const char* optArg);
    };
};

#endif //Config_H