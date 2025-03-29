#include <silver/jlog/logger.h>

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
        clear_files();
    }

    ~logger_test_fixture()
    {
        clear_files();
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

    private:
        void clear_files()
        {
            boost::filesystem::remove_all(logger.working_directory());
            boost::filesystem::remove_all(logger::DEFAULT_WORKING_DIRECTORY);
        }
};