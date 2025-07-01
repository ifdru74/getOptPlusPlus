//
// Created by shtykov on 6/26/25.
//
#include <cstdlib>
#include "appConfig.h"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
using namespace app::config;
std::unique_ptr<Config> Config::instance; // the only instance
std::once_flag Config::initFlag; // instance protection flag
/**
 * converts string into an integer value and stores it to the config
 * @param str1 command argument string
 * @param base 10 for human-readable integers or 16 for less readable integers
 * @param defVal default value
 * @throws std::invalid_argument when conversion impossible
 */
template <typename T>
app::arguments::Value setIntTempl(const char* optArg, const int base, const app::arguments::DefaultValue& defVal) {

    if (optArg !=nullptr && strlen(optArg)>0) {
        if (base==16) {
            if (optArg[0] == '0' && (optArg[1] == 'x' || optArg[1] == 'X')) {
                optArg += 2;
            }
        }
        std::istringstream iss(optArg);
        iss.exceptions(std::istream::failbit | std::istream::badbit);
        if (base == 16) {
            iss >> std::hex;
        }
        try
        {
            T ret;
            iss >> ret;
            return app::arguments::Value(ret);
        }
        catch (const std::exception&)
        {
            throw std::invalid_argument("setIntTempl: invalid value");
        }
    }
    else {
        if (defVal.has_value()) {
            return defVal.value();
        }
    }
    throw std::invalid_argument("setIntTempl: invalid value");
}

/**
 * extracts double value from string
 * @param str1 string containing double value
 * @param defVal default value
 * @return extracted value, default value
 * @throws std::invalid_argument when conversion impossible
 */
static app::arguments::Value setDouble(const char* str1, const app::arguments::DefaultValue& defVal) {
    if (str1!=nullptr) {
        char* pEnd = nullptr;
        const double ret = strtod(str1, &pEnd);
        if (pEnd - str1>0) {
            return ret;
        }
    }
    else {
        if (defVal.has_value()) {
            return defVal.value();
        }
    }
    throw std::invalid_argument("setDouble: invalid value");
}

/**
 * extracts string value
 * @param str1 passed value
 * @param defVal default value
 * @return extracted string
 * @throws std::invalid_argument when extraction failed
 */
static app::arguments::Value setString(const char* str1, const app::arguments::DefaultValue& defVal) {
    if (str1!=nullptr) {
        return str1;
    }
    else {
        if (defVal.has_value()) {
            return defVal.value();
        }
    }
    throw std::invalid_argument("setString: invalid value");
}

/**
 * acquires value from C-string, according to the configured options
 * @param optArg input parameter
 * @return extracted value
 * @throws std::invalid_argument when required parameter is missing
 */
app::arguments::Value app::arguments::Options::acquireValue(const char* optArg) const {
    if (optArg == nullptr) {
        if (type==Type::Mandatory) {
            throw std::invalid_argument("Required option not provided");
        }
        if (defaultValue.has_value()) {
            return defaultValue.value();
        }
        else {
            return true;
        }
    }
    else {
        switch (valueType) {
            case app::arguments::ValueType::Int32:
                return  setIntTempl<int32_t>(optArg, 10, defaultValue);
            case app::arguments::ValueType::Int32h:
                return setIntTempl<int32_t>(optArg, 16, defaultValue);
            case app::arguments::ValueType::Int64:
                return setIntTempl<int64_t>(optArg, 10, defaultValue);
            case app::arguments::ValueType::Int64h:
                return setIntTempl<int64_t>(optArg, 16, defaultValue);
            case app::arguments::ValueType::Float:
                return setDouble(optArg,defaultValue);
            default:
            case app::arguments::ValueType::String:
                return setString(optArg, defaultValue);
        }
    }
    //throw std::invalid_argument("acquireValue: invalid value");
}

/**
 * fills configuration with parameter value from input C-String
 * @param idx options index
 * @param optArg input C-String
 * @param optMap options vector
 * @throws std::invalid_argument when options does not meet input
 */
void Config::fillIn(const app::arguments::Options& opt, const char* optArg) {
    switch (opt.type) {
        case app::arguments::Type::None:
            config[opt.name] = true;
            break;
        case app::arguments::Type::Optional:
            try {
                config[opt.name] = opt.acquireValue(optArg);
            } catch (const std::invalid_argument&) {
                config[opt.name] = true;
            }
            break;
        case app::arguments::Type::Mandatory:
            config[opt.name] = opt.acquireValue(optArg);
            break;
    }
}

/**
 * parses classic command line arguments from main() function
 * @param argc argument count
 * @param argv argument values
 * @param optMap options to parse command line
 * @return parse result
 */
ParseResult Config::parseArgs(int argc, char** argv, const std::vector<app::arguments::Options>& optMap) {
    config.clear();
    size_t i;
    size_t argCount = argc;
    size_t count = optMap.size();
    std::shared_ptr<char> pBuf(new char[count]);
    std::map<std::string, size_t> longMap;
    lastParsed = badArgumentIndex = badArgumentOffset = 0;

    for (i=0; i<count; i++) {
        pBuf.get()[i] = optMap[i].shortCut;
        longMap[optMap[i].name] = i;
        // populates configuration with mandatory parameters defaut values
        if (optMap[i].type == app::arguments::Type::Mandatory && optMap[i].defaultValue.has_value()) {
            config[optMap[i].name] = optMap[i].defaultValue.value();
        }
    }
    for (i=1; i<argCount; i++) {
        if (!argv[i]) {
            badArgumentIndex = i;
            return ParseResult::Error;
        }
        if (argv[i][0]=='-') {
            if (argv[i][1]=='-') {
                if (argv[i][2] == '\0') {
                    // have just received '--' argument, next arguments won't be parsed
                    lastParsed = i;
                    break;
                }
                // long option
                try {
                    size_t idx = longMap.at(argv[i]+2);
                    const auto& rOpt = optMap[idx];
                    if (rOpt.type!=app::arguments::Type::None) {
                        if (i+1<argc && argv[i+1][0]!='-') {
                            i ++;
                            fillIn(rOpt, argv[i]);
                        }
                        else {
                            if (rOpt.type == app::arguments::Type::Mandatory) {
                                return ParseResult::Error;
                            }
                            else {
                                config[rOpt.name] = true;
                            }
                        }
                    }
                    else {
                        fillIn(rOpt, nullptr);
                    }
                } catch (const std::out_of_range&) {
                    badArgumentIndex = i;
                    return ParseResult::BadOptionIndex;

                } catch (const std::invalid_argument&) {
                    badArgumentIndex = i;
                    return ParseResult::Error;
                }
            }
            else {
                // short option(s)
                const char* ptr = argv[i] + 1;
                while (*ptr!='\0') {
                    const char* p = static_cast<const char*>(::memchr(pBuf.get(), *ptr, count));
                    if (p==nullptr) {
                        badArgumentIndex = i;
                        badArgumentOffset = ptr - argv[i];
                        return ParseResult::Unknown;
                    }
                    size_t idx = p - pBuf.get();
                    try {
                        const auto& rOpt = optMap[idx];
                        if (*(ptr+1)=='\0') {
                            if (i+1<argc && argv[i + 1][0] != '-') {
                                i ++;
                                fillIn(rOpt, argv[i]);
                            }
                            else {
                                if (rOpt.type == app::arguments::Type::Mandatory) {
                                    return ParseResult::Error;
                                }
                                else {
                                    config[rOpt.name] = true;
                                }
                            }
                        }
                        else {
                            fillIn(rOpt, nullptr);
                        }
                    } catch (const std::out_of_range&) {
                        badArgumentIndex = i;
                        return ParseResult::BadOptionIndex;
                    } catch (const std::invalid_argument&) {
                        badArgumentIndex = i;
                        return ParseResult::Error;
                    }
                    ptr ++;
                }
            }
        }
    }
    return ParseResult::Parsed;
}

/**
 * a non-throwing test for parameter availability
 * @param name a configuration parameter name
 * @return true, if there is a value
 */
bool Config::hasValue(const std::string& name) const noexcept {
    return  config.count(name) > 0;
}

/**
 * returns paramenter value
 * @param name a configuration parameter name
 * @return value
 * @throws std::out_of_range when no parameter available
 */
app::arguments::Value Config::getValue(const std::string& name) const {
    return config.at(name);
}

/**
 * prints usage help message
 * @param optMap a configuration parameters options
 * @param progName - program name to be displayed
 * @param out - an output stream receives the usage message
 * @return the output stream resulting state
 */
std::ostream& Config::printHelp(const std::vector<app::arguments::Options>& optMap, const std::string& progName, std::ostream& out) {
    size_t pos = progName.rfind(std::filesystem::path::preferred_separator);
    out << std::endl << "Usage: " << std::endl;
    if (pos!=std::string::npos) {
        out << progName.substr(pos+1);
    }
    else {
        out << progName;
    }
    out << " [options]" << std::endl << "Options:" << std::endl;
    for (const auto& opt: optMap) {
        out << '\t';
        if (!::isspace(opt.shortCut) && opt.shortCut!='\0') {
            out << '-' << opt.shortCut << " or ";
        }
        out << "--" << opt.name << " " << opt.desc;
        if (opt.defaultValue.has_value()) {
            out << ", default: ";
            const auto& val = opt.defaultValue.value();
            switch (opt.valueType) {
                case app::arguments::ValueType::String:
                    out << std::get<std::string>(val);
                    break;
                case app::arguments::ValueType::Int32:
                    out << std::get<int32_t>(val);
                    break;
                case app::arguments::ValueType::Int32h:
                    out << "0x" << std::hex <<  std::get<int32_t>(val);
                    break;
                case app::arguments::ValueType::Float:
                    out << std::get<double>(val);
                    break;
                case app::arguments::ValueType::Int64:
                    out << "0x" << std::hex << std::get<int64_t>(val);
                    break;
                case app::arguments::ValueType::Int64h:
                    out << std::get<int64_t>(val);
                    break;
                default:
                    out << '?';
                    break;
            }
        }
        out << std::endl;
    }
    return out;
}

/**
 * validates that configuration contains mandatory parameters
 * @param optMap a configuration parameters options
 * @return absent mandatory parameter option index or std::string::npos when all mandatory parameters have their values
 */
size_t app::config::Config::valid(const std::vector<app::arguments::Options>& optMap)
{
    for (size_t i = 0; i < optMap.size(); i++) {
        if (optMap[i].type != app::arguments::Type::Mandatory) {
            continue; // if argument not mandatory - skip it
        }
        try
        {
            const auto& cfg = config.at(optMap[i].name);
            if (optMap[i].defaultValue.has_value()) {
                if (cfg.index() != optMap[i].defaultValue.value().index()) {
                    return i; // value type not equal to default value type
                }
            }
        }
        catch (const std::out_of_range&)
        {
            return i; // argument not provided
        }
        catch (const std::exception&)
        {
            return i; // generic issue
        }
    }
    // all required arguments were provided
    return MANDATORY_PASSED_ALL;
}
