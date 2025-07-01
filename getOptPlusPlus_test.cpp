// getOptPlusPlus.cpp : Defines the entry point for the application.
//

#include "getOptPlusPlus.h"
#include "appConfig.h"
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;
std::vector<app::arguments::Options> optMap = {
    app::arguments::Options('h', "help", "displays help usage message"),
    app::arguments::Options('O', "output_directory", "a directory where received files are stored", app::arguments::Type::Mandatory, app::arguments::ValueType::String),
    app::arguments::Options('D', "dark_color", "a color to detect dark images", app::arguments::Type::Mandatory, app::arguments::ValueType::Int32h, app::arguments::DefaultValue(int32_t(0x090909))),
    app::arguments::Options('I', "id", "an id for something", app::arguments::Type::Optional, app::arguments::ValueType::Int64, app::arguments::DefaultValue(int64_t(0x100))),
    app::arguments::Options('L', "log_dir","A directory to store program logs", app::arguments::Type::Mandatory, app::arguments::ValueType::String, app::arguments::DefaultValue(".")),
    app::arguments::Options('P', "probability", "Probability to match file", app::arguments::Type::Mandatory, app::arguments::ValueType::Float, app::arguments::DefaultValue(0.5)),
};
constexpr const int EXIT_HELP = EXIT_FAILURE + 1;
constexpr const int EXIT_MANDATORY = EXIT_FAILURE + 2;
constexpr const int EXIT_ABSENT = EXIT_FAILURE + 3;
constexpr const int EXIT_DIFF_TYPE = EXIT_FAILURE + 4;
constexpr const int EXIT_GENERIC = EXIT_FAILURE + 5;

static std::ostream& printValue(app::arguments::Value val, std::ostream& os)
{
    switch (val.index()) {
    case 0: // string
        os << std::get<std::string>(val);
        break;
    case 1: // int32_t
        os << std::get<int32_t>(val);
        break;
    case 2: // int64_t
        os << std::get<int64_t>(val);
        break;
    case 3: // int64_t
        os << std::get<double>(val);
        break;
    case 4: // int64_t
        if (std::get<bool>(val)) {
            os << "true";
        }
        else {
            os << "false";
        }
        break;
    default:
        os << "unknown value type";
    }
    return os;
}

static int testSet(int argc, char* argv[], bool bPrintUsage = false)
{
    app::config::Config& cfg = app::config::Config::getInstance();
    int nRet = EXIT_SUCCESS;
    const app::config::ParseResult res = cfg.parseArgs(argc, argv, optMap);
    bool bHelpRequired = false;
    switch (res) {
    case app::config::ParseResult::Parsed:
        bHelpRequired = cfg.hasValue("help");
        break;
    case app::config::ParseResult::Error:
        std::cerr << "unable to parse command line parameters" << std::endl;
        nRet = EXIT_FAILURE;
        break;
    case app::config::ParseResult::HelpRequired:
        std::cerr << "help usage required" << std::endl;
        bHelpRequired = true;
        nRet = EXIT_FAILURE;
        break;
    case app::config::ParseResult::Unknown:
        std::cerr << "unknown option passed. Error in argument at index " << cfg.badArgumentIndex << " (offset in argument " << cfg.badArgumentOffset << ")" << std::endl;
        bHelpRequired = true;
        nRet = EXIT_FAILURE;
        break;
    case app::config::ParseResult::BadOptionIndex:
        std::cerr << "unable to compute proper Options index. Error in argument at index " << cfg.badArgumentIndex << " (offset in argument " << cfg.badArgumentOffset << ")" << std::endl;
        bHelpRequired = true;
        nRet = EXIT_FAILURE;
        break;
    case app::config::ParseResult::None:
    default:
        bHelpRequired = true;
        std::cerr << "an unknown parsing state detected" << std::endl;
        nRet = EXIT_FAILURE;
        break;
    }
    if (nRet != EXIT_SUCCESS) {
        if (bPrintUsage) {
            app::config::Config::printHelp(optMap, argv[0], std::cout);
        }
    }
    if (bHelpRequired) {
        return EXIT_HELP;
    }
    if (nRet == EXIT_SUCCESS) {
        if (cfg.lastParsed != 0) {
            std::cout << "There are unparsed arguments from " << cfg.lastParsed + 1 << std::endl;
        }
        size_t idx = cfg.valid(optMap);
        if (idx != app::config::Config::MANDATORY_PASSED_ALL) {
            std::cerr << "mandatory value for '" << optMap[idx].name << "' was not provided" << std::endl;
            return EXIT_MANDATORY;
        }
        bool bFailed = false;
        for (const auto& opt : optMap) {
            try {
                const auto& param = cfg.getValue(opt.name);
                if (opt.defaultValue.has_value()) {
                    if (opt.defaultValue.value().index() == param.index()) {
                        std::cout << opt.name << " => '";
                        printValue(param, std::cout) << "'" << std::endl;
                        continue;
                    }
                    else {
                        std::cerr << "Different value types for '" << opt.name << "' expected: " << opt.defaultValue.value().index() << ", actual: " << param.index() << std::endl;
                        bFailed = true;
                    }
                }
                std::cout << opt.name << " => '";
                printValue(param, std::cout) << "'" << std::endl;
                if (bFailed) {
                    return EXIT_DIFF_TYPE;
                }
            }
            catch (const std::out_of_range& er) {
                if (opt.type == app::arguments::Type::Mandatory) {
                    std::cerr << er.what() << ": no value for '" << opt.name << "'" << std::endl;
                    return EXIT_ABSENT;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "generic failure: " << e.what() << std::endl;
                return EXIT_GENERIC;
            }
        }
    }
    return nRet;

}

const char* testSet1[]{ "program1" };
const char* testSet2[]{ "program1", "-h"};
const char* testSet31[]{ "program1", "-help" };
const char* testSet32[]{ "program1", "--help" };
const char* testSet41[]{ "program1", "-D"};
const char* testSet42[]{ "program1", "-D", "505050"};
const char* testSet5[]{ "program1", "-D", "505050", "-O", "."};
const char* testSet6[]{ "program1", "-D", "505050", "-O", ".", "-I","1"};
const char* testSet7[]{ "program1", "-D", "505050", "-O", ".", "-P","0"};
const char* testSet8[]{ "program1", "-D", "505050", "-O", ".", "-P","0.7"};
std::map<int, std::string> stringTable = {
    {EXIT_SUCCESS, "EXIT_SUCCESS"},
    {EXIT_FAILURE, "EXIT_FAILURE"},
    {EXIT_HELP, "EXIT_HELP"},
    {EXIT_MANDATORY, "EXIT_MANDATORY"},
    {EXIT_ABSENT, "EXIT_ABSENT"},
    {EXIT_DIFF_TYPE, "EXIT_DIFF_TYPE"},
    {EXIT_GENERIC, "EXIT_GENERIC"},
};
int performAllTests()
{
    int testNum = 1;
    int expected = EXIT_MANDATORY;
    std::cout << "Performing test " << testNum << std::endl;
    int nRet = testSet(1, (char**)(testSet1));
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    testNum = 2;
    expected = EXIT_HELP;
    std::cout << "Performing test " << testNum << std::endl;
    nRet = testSet(2, (char**)testSet2);
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    testNum = 31;
    expected = EXIT_HELP;
    std::cout << "Performing test " << testNum << std::endl;
    nRet = testSet(2, (char**)testSet31);
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    testNum = 32;
    expected = EXIT_HELP;
    std::cout << "Performing test " << testNum << std::endl;
    nRet = testSet(2, (char**)testSet32);
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    testNum = 41;
    expected = EXIT_FAILURE;
    std::cout << "Performing test " << testNum << std::endl;
    nRet = testSet(2, (char**)testSet41);
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    testNum = 42;
    expected = EXIT_MANDATORY;
    std::cout << "Performing test " << testNum << std::endl;
    nRet = testSet(3, (char**)testSet42);
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    testNum = 5;
    expected = EXIT_SUCCESS;
    std::cout << "Performing test " << testNum << std::endl;
    nRet = testSet(5, (char**)testSet5);
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    testNum = 6;
    expected = EXIT_SUCCESS;
    std::cout << "Performing test " << testNum << std::endl;
    nRet = testSet(7, (char**)testSet6);
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    testNum = 7;
    expected = EXIT_SUCCESS;
    std::cout << "Performing test " << testNum << std::endl;
    nRet = testSet(7, (char**)testSet7);
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    testNum = 8;
    expected = EXIT_SUCCESS;
    std::cout << "Performing test " << testNum << std::endl;
    nRet = testSet(7, (char**)testSet8);
    if (nRet == expected) {
        std::cout << "Test " << testNum << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << testNum << " failed. Expected " << stringTable[expected] << ", actual " << stringTable[nRet] << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    app::config::Config& cfg = app::config::Config::getInstance();
    int nRet = EXIT_SUCCESS;
    if (argc == 1) {
        return performAllTests();
    }
    //return testSet(argc, argv, true);
    std::istringstream iss(argv[1]);
    int i;
    iss.exceptions(istream::failbit | istream::badbit);
    try
    {
        iss >> std::hex;
        iss >> i;
        std::cout << i << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}
