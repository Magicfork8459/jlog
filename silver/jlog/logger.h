#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <regex>

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/keywords/format.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/describe.hpp>
#include <boost/type_index.hpp>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#define LOGGER_UNUSED(expr) (void)(expr);

using text_sink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;
using text_sink_ptr = boost::shared_ptr<text_sink>;
using file_sink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>;
using file_sink_ptr = boost::shared_ptr<file_sink>;

namespace silver
{
    namespace jlog
    {
        BOOST_DEFINE_ENUM
        (
            severity_levels,
            //NOTE Indicates the system is completely unusable
            emergency,
            //NOTE Indicates an action needs to be taken to prevent something from happening
            alert,
            //NOTE Hardware failure or other critical errors
            critical,
            //NOTE Application error
            error,
            //NOTE Something didn't work that didn't cause an application error, but could cause one at some point
            warning,
            //NOTE Information about unusual events but are not specifically a problem
            notice,
            //NOTE Positive information about stuff working
            informational,
            //NOTE Information only useful for debug purposes
            debug
        )

        //NOTE Globally override this operator so the logger will all our severity strings
        template<typename CharT, typename TraitsT>
        inline std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& stream, severity_levels level)
        {
            return stream << boost::describe::enum_to_string(level, "");
        }

        using filepath_vector = std::vector<boost::filesystem::path>;

        class logger
        {
            public:
                static const std::string JSON_ATTRIBUTE_SEVERITY;
                static const std::string JSON_ATTRIBUTE_TIMESTAMP;
                static const std::string JSON_ATTRIBUTE_UPTIME;
                static const std::string JSON_ATTRIBUTE_THREAD_ID;
                static const std::string JSON_ATTRIBUTE_PROCESS_ID;
                static const std::string JSON_ATTRIBUTE_PROCESS;
                static const std::string JSON_ATTRIBUTE_SOURCE_FILE;
                static const std::string JSON_ATTRIBUTE_SOURCE_LINE_NUMBER;
                static const std::string JSON_ATTRIBUTE_MESSAGE;
                static const boost::filesystem::path DEFAULT_WORKING_DIRECTORY;
                
                logger();
                logger(const boost::filesystem::path& working_directory);
                logger(const std::string filename, const boost::filesystem::path& working_directory);
                ~logger();

                //#define LOG_MESSAGE(message, level) { std::stringstream _s; _s << message; Singleton->LogMessage(_s.str().c_str(), level, __FILE__, __LINE__); }
                
                static severity_levels severity_level();
                static void severity_level(const severity_levels& severity_level);
                //NOTE Indicates the system is completely unusable
                static void emergency(const std::string& message);
                //NOTE Indicates an action needs to be taken to prevent something from happening
                static void alert(const std::string& message);
                //NOTE Hardware failure or other critical errors
                static void critical(const std::string& message);
                //NOTE Application error
                static void error(const std::string& message);
                //NOTE Something didn't work that didn't cause an application error, but could cause one at some point
                static void warning(const std::string& message);
                //NOTE Information about unusual events but are not specifically a problem
                static void notice(const std::string& message);
                //NOTE Positive information about stuff working
                static void informational(const std::string& message);
                //NOTE Information only useful for debug purposes
                static void debug(const std::string& message);
                static size_t references();
                static void references(const size_t& value);
                static filepath_vector targets();
                static void rotate();
                //NOTE Set the maximum log file size to a different value            
                static void file_size(const size_t& value);
                static size_t file_size();
                static void working_directory(const boost::filesystem::path& value);
                static boost::filesystem::path working_directory();
                static void maximum_file_count(const size_t& value);
                static size_t maximum_file_count();
                static boost::filesystem::path current_log_file();
                static void clear_working_directory() noexcept;
                
            protected:
                static void formatter(const boost::log::record_view& recordView, boost::log::formatting_ostream& stream);
            private:          
                static void add_reference();              
                static void file_sink();
                static boost::filesystem::path construct_file_path();
                //NOTE Closes the file and ensures the json will be valid
                //arg[sizeCheck] If this is true, the close operation will only happen if the file has reached it's size limit
                static bool close_out(const boost::filesystem::path& target, bool sizeCheck = true);
                //NOTE Returns either -1 or the rotation index used for this file
                static int get_file_number(const boost::filesystem::path& target);

                static const int INVALID_FILENUMBER;
                static std::regex FILENAME_REGEX;
                static const std::regex FILENUMBER_REGEX;
                static boost::format DEFAULT_LOG_FILENAME_FORMAT;
                static boost::format FILENAME_REGEX_FORMAT;
                //NOTE Track whether the constructor has been called before and how many times            
                static size_t _references;
                static boost::log::sources::severity_logger<severity_levels> _logger;
                static text_sink_ptr _console_sink;   
                static file_sink_ptr _file_sink;
                static size_t _file_size_limit;
                static size_t _file_count_limit;
                static size_t _current_file_rotation_index;
                //static std::set<std::string> _filenames;
                static severity_levels _severity_level;
                static std::string _filename;
                static boost::filesystem::path _working_directory;
        };
        
        #define LOG_SEVERITY(logger, message, severity) {   \
            std::stringstream _s;                           \
            _s << message;                                  \
            switch(severity){                               \
                case severity_levels::emergency:              \
                    logger.emergency(_s.str());             \
                    break;                                  \
                case severity_levels::alert:                  \
                    logger.alert(_s.str());                 \
                    break;                                  \
                case severity_levels::critical:               \
                    logger.critical(_s.str());              \
                    break;                                  \
                case severity_levels::error:                  \
                    logger.error(_s.str());                 \
                    break;                                  \
                case severity_levels::warning:                \
                    logger.warning(_s.str());               \
                    break;                                  \
                case severity_levels::notice:                 \
                    logger.notice(_s.str());                \
                    break;                                  \
                case severity_levels::informational:          \
                    logger.informational(_s.str());         \
                    break;                                  \
                case severity_levels::debug:                  \
                    logger.debug(_s.str());                 \
                    break;                                  \
            }                                               \
        };                                                  \
        
        //NOTE Indicates the system is completely unusable
        #define LOG_EMERGENCY(logger, message) LOG_SEVERITY(logger, message, severity_levels::emergency);
        //NOTE Indicates an action needs to be taken to prevent something from happening
        #define LOG_ALERT(logger, message) LOG_SEVERITY(logger, message, severity_levels::alert);
        //NOTE Hardware failure or other critical errors
        #define LOG_CRITICAL(logger, message) LOG_SEVERITY(logger, message, severity_levels::critical);
        //NOTE Application error
        #define LOG_ERROR(logger, message) LOG_SEVERITY(logger, message, severity_levels::error);
        //NOTE Something didn't work that didn't cause an application error, but could cause one at some point
        #define LOG_WARNING(logger, message) LOG_SEVERITY(logger, message, severity_levels::warning);
        //NOTE Information about unusual events but are not specifically a problem
        #define LOG_NOTICE(logger, message) LOG_SEVERITY(logger, message, severity_levels::notice);
        //NOTE Positive information about stuff working
        #define LOG_INFORMATIONAL(logger, message) LOG_SEVERITY(logger, message, severity_levels::informational);
        //NOTE Information only useful for debug purposes
        #define LOG_DEBUG(logger, message) LOG_SEVERITY(logger, message, severity_levels::debug);
    }    
}