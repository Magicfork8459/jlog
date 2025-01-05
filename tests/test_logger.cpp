#define BOOST_TEST_MODULE "JLog Unit Test Driver"

#include <iso646.h>

#include <boost/test/unit_test.hpp>
#include <boost/json.hpp>

#include "logger.h"

//TODO test changing severity level
//TODO config file support

BOOST_AUTO_TEST_SUITE(logger_test_suite)

using namespace silver::jlog;

struct test_record
{
    severity_levels severity;
    
    std::string message;
};

struct logger_test_fixture
{
    inline static const std::string MESSAGE_EMERGENCY = "EMERGENCY";
    inline static const std::string MESSAGE_ALERT = "ALERT";
    inline static const std::string MESSAGE_CRITICAL = "CRITICAL";
    inline static const std::string MESSAGE_ERROR = "ERROR";
    inline static const std::string MESSAGE_WARNING = "WARNING";
    inline static const std::string MESSAGE_NOTICE = "NOTICE";
    inline static const std::string MESSAGE_INFORMATIONAL = "INFORMATIONAL";
    inline static const std::string MESSAGE_DEBUG = "DEBUG";
    
    std::map<unsigned short, test_record> test_records =
    {
        { 0, { severity_levels::emergency, MESSAGE_EMERGENCY }},
        { 1, { severity_levels::alert, MESSAGE_ALERT }},        
        { 2, { severity_levels::critical, MESSAGE_CRITICAL }},
        { 3, { severity_levels::error, MESSAGE_ERROR }},
        { 4, { severity_levels::warning, MESSAGE_WARNING }},
        { 5, { severity_levels::notice, MESSAGE_NOTICE }},
        { 6, { severity_levels::informational, MESSAGE_INFORMATIONAL }},
        { 7, { severity_levels::debug, MESSAGE_DEBUG }}
    };

    logger_test_fixture()
    {
        boost::filesystem::remove_all(logger.working_directory());
    }
    
    void confirm_severity(const severity_levels& severity, const std::string& contents)
    {
        boost::mp11::mp_for_each<boost::describe::describe_enumerators<severity_levels>>
        (
            [&](auto description)
            {
                if(description.value <= severity)
                {
                    BOOST_TEST_REQUIRE(contents.contains(description.name));
                }
                else
                {
                    BOOST_TEST_REQUIRE(not contents.contains(description.name));
                }
            }
        );
    }

    void logtest_records()
    {
        for(auto&& record : test_records)
        {
            BOOST_TEST_MESSAGE("Logging test " << boost::describe::enum_to_string(record.second.severity, "") << " message...");

            switch(record.second.severity)
            {
                case severity_levels::emergency:
                    logger.emergency(record.second.message);
                    break;
                case severity_levels::alert:
                    logger.alert(record.second.message);
                    break;
                case severity_levels::critical:
                    logger.critical(record.second.message);
                    break;
                case severity_levels::error:
                    logger.error(record.second.message);
                    break;
                case severity_levels::warning:
                    logger.warning(record.second.message);
                    break;
                case severity_levels::notice:
                    logger.notice(record.second.message);
                    break;
                case severity_levels::informational:
                    logger.informational(record.second.message);
                    break;
                case severity_levels::debug:
                    logger.debug(record.second.message);
                    break;
            }
        }
    }

    logger logger;
};

//NOTE Test Multiple Files C#69
//TODO figure out how to test the console logger
//TODO test the macros

// //TODO improve this later with reflection stuff
BOOST_FIXTURE_TEST_CASE(FileLogger, logger_test_fixture)
{
    logtest_records();

    BOOST_TEST_MESSAGE("Rotating the log file...");
    //NOTE Grab this here because the current file reference will change after the forced rotation
    auto target = logger.targets()[0];
    //NOTE Rotate so the file will closed out and be valid json
    logger.rotate();
    
    BOOST_TEST_MESSAGE("Reading in the log file...");
    std::ifstream input(target.string());
    std::stringstream inputStream;
    {
        inputStream << input.rdbuf();
    }

    BOOST_TEST_MESSAGE("Parsing the log file contents as json...");
    boost::json::object parsed = boost::json::parse(inputStream).get_object();
        
    unsigned short recordCount = 0;
    for(auto it = parsed.begin(); it != parsed.end(); ++it)
    {
        boost::json::object subObject = it->value().get_object();
        BOOST_TEST_MESSAGE("Converting the record id to an integer value...");
        unsigned short recordNumber = std::stoi(it->key());        
        std::string severityString = &subObject.at(logger::JSON_ATTRIBUTE_SEVERITY).as_string()[0];
        severity_levels severity = severity_levels::alert;
        {
            boost::describe::enum_from_string(severityString, severity);
        }
        
        std::string timestampString = &subObject.at(logger::JSON_ATTRIBUTE_TIMESTAMP).as_string()[0];
        std::string uptimeString = &subObject.at(logger::JSON_ATTRIBUTE_UPTIME).as_string()[0];
        std::string threadIDString = &subObject.at(logger::JSON_ATTRIBUTE_THREAD_ID).as_string()[0];
        std::string processIDString = &subObject.at(logger::JSON_ATTRIBUTE_PROCESS_ID).as_string()[0];
        std::string processString = &subObject.at(logger::JSON_ATTRIBUTE_PROCESS).as_string()[0];
        std::string messageString = &subObject.at(logger::JSON_ATTRIBUTE_MESSAGE).as_string()[0];
        
        
        BOOST_TEST_MESSAGE("Requiring that the record id is the expected value...");
        BOOST_TEST_REQUIRE(recordNumber == recordCount);
        
        BOOST_TEST_MESSAGE("Requiring that the severity string exists...");
        BOOST_TEST_REQUIRE((not severityString.empty()));
        BOOST_TEST_MESSAGE("Requiring that the severity string has the correct content...");
        BOOST_TEST_REQUIRE((severity == test_records.at(recordCount).severity));
        BOOST_TEST_MESSAGE("Requiring that the timestamp string exists...");
        BOOST_TEST_REQUIRE((not timestampString.empty()));
        BOOST_TEST_MESSAGE("Requiring that the uptime string exists...");
        BOOST_TEST_REQUIRE((not uptimeString.empty()));
        BOOST_TEST_MESSAGE("Requiring that the thread id string exists...");
        BOOST_TEST_REQUIRE((not threadIDString.empty()));
        BOOST_TEST_MESSAGE("Requiring that the process id string exists...");
        BOOST_TEST_REQUIRE((not processIDString.empty()));
        BOOST_TEST_MESSAGE("Requiring that the process string exists...");
        BOOST_TEST_REQUIRE((not processString.empty()));
        BOOST_TEST_MESSAGE("Requiring that the message string exists...");
        BOOST_TEST_REQUIRE((not messageString.empty()));
        BOOST_TEST_MESSAGE("Requiring that the message string has the correct content...");
        BOOST_TEST_REQUIRE((messageString.compare(test_records.at(recordCount).message) == 0));
        
        std::cout << it->key() << ':' << it->value() << std::endl;
        ++recordCount;
    }
}

BOOST_FIXTURE_TEST_CASE(severity_levelFilter, logger_test_fixture)
{
    boost::mp11::mp_for_each<boost::describe::describe_enumerators<severity_levels>>(
        [&](auto description)
        {
            logger.severity_level(description.value);
            
            BOOST_TEST_REQUIRE(logger.severity_level() == description.value);
            
            logtest_records();
            
            boost::filesystem::path targetPath(logger.targets()[0]);
            std::ifstream targetFile(targetPath.string());
            logger.rotate();
            std::stringstream contents;
            {
                contents << targetFile.rdbuf();
                targetFile.close();
            }
            boost::filesystem::remove(targetPath);
            
            confirm_severity(description.value, contents.str());
        });

    boost::mp11::mp_for_each<boost::mp11::mp_reverse<boost::describe::describe_enumerators<severity_levels>>>(
        [&](auto description)
        {
            logger.severity_level(description.value);
            
            BOOST_TEST_REQUIRE(logger.severity_level() == description.value);
            
            logtest_records();
            
            boost::filesystem::path targetPath(logger.targets()[0]);
            std::ifstream targetFile(targetPath.string());
            logger.rotate();
            std::stringstream contents;
            {
                contents << targetFile.rdbuf();
                targetFile.close();
            }
            boost::filesystem::remove(targetPath);
            
            confirm_severity(description.value, contents.str());
        });
}

BOOST_FIXTURE_TEST_CASE(FileRotation, logger_test_fixture, * boost::unit_test_framework::timeout(5))
{
    logger.file_size(1024);
    const size_t maxRotations = logger.maximum_file_count();
    bool check = true;
    size_t totalSize = 0;
    size_t maxSize = logger.file_size() * logger.maximum_file_count();
    size_t lowerBound = logger.file_size() / 2;
    size_t upperBound = static_cast<float>(logger.file_size() * 1.5f);
    boost::filesystem::directory_iterator it;
    
    do
    {
        logtest_records();
        
        it = boost::filesystem::directory_iterator(logger.working_directory()); //NOTE don't need to set this every time, but it doesn't really hurt anything either
        totalSize = 0;

        for(auto&& entry : it)
        {            
            std::string pathAsString = entry.path().string();
            size_t fileSize = 0;
            std::ifstream input(pathAsString, std::ios_base::binary | std::ios_base::ate);
            BOOST_REQUIRE(input.is_open());
            fileSize = input.tellg();
            totalSize += fileSize;
            input.close();
            
            check = !(totalSize >= maxSize);
        }
        
    } while (check);
    
    for(auto&& entry: it)
    {
        std::string pathAsString = entry.path().string();
        size_t fileSize = 0;
        std::ifstream input(pathAsString, std::ios_base::binary | std::ios_base::ate);
        BOOST_REQUIRE(input.is_open());
        fileSize = input.tellg();            
        input.close();

        BOOST_TEST_MESSAGE(boost::str(boost::format("Requiring that %1% of size %2% is between %3% and %4%") % pathAsString % fileSize % lowerBound % upperBound));
        BOOST_REQUIRE((fileSize <= upperBound and fileSize >= lowerBound));
    }
    
    for(size_t i = 0; i < maxRotations; ++i)
    {
        logger.informational("rotating");
        logger.rotate();
        logger.informational("rotated");
    }

    size_t fileCount = 0;
    for(auto&& entry: it)
    {
        fileCount += entry.is_regular_file();
    }

    BOOST_REQUIRE(fileCount <= maxRotations);    
}

BOOST_AUTO_TEST_SUITE_END()