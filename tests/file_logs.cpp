#define BOOST_TEST_MODULE "JLog Unit Test Driver"

#include <iso646.h>

#include <boost/test/unit_test.hpp>
#include <boost/json.hpp>

#include <silver/jlog/logger.h>
#include "fixture.hpp"

//TODO test changing severity level
//TODO config file support

BOOST_AUTO_TEST_SUITE(logger_test_suite)

//NOTE Test Multiple Files C#69
//TODO figure out how to test the console logger
//TODO test the macros

// //TODO improve this later with reflection stuff
BOOST_FIXTURE_TEST_CASE(file_logger, logger_test_fixture)
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
        
        BOOST_TEST_REQUIRE(recordNumber == recordCount);
        BOOST_TEST_REQUIRE((not severityString.empty()));
        BOOST_TEST_REQUIRE((severity == test_records.at(recordCount).severity));
        BOOST_TEST_REQUIRE((not timestampString.empty()));
        BOOST_TEST_REQUIRE((not uptimeString.empty()));
        BOOST_TEST_REQUIRE((not threadIDString.empty()));
        BOOST_TEST_REQUIRE((not processIDString.empty()));
        BOOST_TEST_REQUIRE((not processString.empty()));
        BOOST_TEST_REQUIRE((not messageString.empty()));
        BOOST_TEST_REQUIRE((messageString.compare(test_records.at(recordCount).message) == 0));
        
        std::cout << it->key() << ':' << it->value() << std::endl;
        ++recordCount;
    }
}

BOOST_FIXTURE_TEST_CASE(severity_level_filter, logger_test_fixture)
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

BOOST_FIXTURE_TEST_CASE(file_rotation, logger_test_fixture, * boost::unit_test_framework::timeout(5))
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