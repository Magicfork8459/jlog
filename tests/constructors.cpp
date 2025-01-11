#define BOOST_TEST_MODULE "JLog Constructors"

#include <iso646.h>

#include <boost/test/unit_test.hpp>
#include <boost/json.hpp>

#include "logger.h"

//TODO test changing severity level
//TODO config file support

BOOST_AUTO_TEST_SUITE(jlog_tests_constructors)

using namespace silver::jlog;

const std::string TEST_WORKING_DIRECTORY = "C:\\test_jlog";
const std::string TEST_FILENAME = "test_log_file.json";
const std::string EXPECTED_FILENAME = "test_log_file_1.json";

BOOST_AUTO_TEST_CASE(construct_working_directory_filepath)
{
    logger logger(TEST_FILENAME, TEST_WORKING_DIRECTORY);
    logger.informational("Test");   

    auto current_file = logger.current_log_file();
    std::string current_filename = current_file.filename().string();

    BOOST_TEST_REQUIRE(boost::filesystem::exists(current_file));
    BOOST_TEST_REQUIRE(current_filename.compare(EXPECTED_FILENAME) == 0);
    BOOST_TEST_REQUIRE(logger.working_directory().compare(TEST_WORKING_DIRECTORY) == 0);
    BOOST_TEST_REQUIRE(current_file.size() > 0);

    try
    {        
        logger.clear_working_directory();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

BOOST_AUTO_TEST_SUITE_END()