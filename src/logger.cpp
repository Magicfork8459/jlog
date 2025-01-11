#include "logger.h"

//TODO configuration stuff
//TODO log to syslog server

namespace silver
{
    namespace jlog
    {
        const std::string logger::JSON_ATTRIBUTE_SEVERITY("severity");
        const std::string logger::JSON_ATTRIBUTE_TIMESTAMP("timestamp");
        const std::string logger::JSON_ATTRIBUTE_UPTIME("uptime");
        const std::string logger::JSON_ATTRIBUTE_THREAD_ID("thread_id");
        const std::string logger::JSON_ATTRIBUTE_PROCESS_ID("process_id");
        const std::string logger::JSON_ATTRIBUTE_PROCESS("process");
        const std::string logger::JSON_ATTRIBUTE_SOURCE_FILE("source_file");
        const std::string logger::JSON_ATTRIBUTE_SOURCE_LINE_NUMBER("source_line");
        const std::string logger::JSON_ATTRIBUTE_MESSAGE("message");
        const boost::filesystem::path logger::DEFAULT_WORKING_DIRECTORY(boost::filesystem::temp_directory_path().append("/jlog"));

        size_t logger::_references = 0;
        text_sink_ptr logger::_console_sink = NULL;
        file_sink_ptr logger::_file_sink = NULL;    
        size_t logger::_file_count_limit = 3;
        size_t logger::_file_size_limit = 650000000 / logger::_file_count_limit; //NOTE Want to be able to fit the default stuff onto at least 1 compact disc
        size_t logger::_current_file_rotation_index = 1;
        severity_levels logger::_severity_level = severity_levels::informational;
        boost::log::sources::severity_logger<severity_levels> logger::_logger = {};    
        //std::set<std::string> Logger::_filenames = {};
        boost::filesystem::path logger::_working_directory = DEFAULT_WORKING_DIRECTORY;
        
        const int logger::INVALID_FILENUMBER = -1;    
        boost::format logger::FILENAME_REGEX_FORMAT = boost::format(".*%1%_[0-9]*.json$");
        boost::format logger::DEFAULT_LOG_FILENAME_FORMAT = boost::format("%1%_%2%.json");
        std::string logger::_filename = "log";
        std::regex logger::FILENAME_REGEX(boost::str( FILENAME_REGEX_FORMAT % logger::_filename));
        const std::regex logger::FILENUMBER_REGEX("[0-9]*.json$");

        BOOST_LOG_ATTRIBUTE_KEYWORD(_severity, "Severity", severity_levels);
        BOOST_LOG_ATTRIBUTE_KEYWORD(_timeStamp, "TimeStamp", boost::log::attributes::utc_clock::value_type);
        BOOST_LOG_ATTRIBUTE_KEYWORD(_uptime, "Uptime", boost::log::attributes::timer::value_type);
        BOOST_LOG_ATTRIBUTE_KEYWORD(_threadID, "ThreadID", boost::log::attributes::current_thread_id::value_type);
        BOOST_LOG_ATTRIBUTE_KEYWORD(_processID, "ProcessID", boost::log::attributes::current_process_id::value_type);
        BOOST_LOG_ATTRIBUTE_KEYWORD(_process, "Process", boost::log::attributes::current_process_name::value_type);
        BOOST_LOG_ATTRIBUTE_KEYWORD(_lineID, "LineID", size_t);

        void logger::formatter(const boost::log::record_view& recordView, boost::log::formatting_ostream& stream)
        {
            auto file = _file_sink->locked_backend()->get_current_file_name();
            bool emptyFile = file.empty();
            
            if(emptyFile)
            { //NOTE This should be the first line since the file doesn't exist yet, so write the beginning of our root json object
                stream << "{\n";
            }
            else
            { //NOTE Write a comma to maintain json validity as there should be at least 1 record contained within the file now
                stream << ",\n";
            }

            //NOTE Write record
            stream << "\t\"" << boost::log::extract<size_t>("LineID", recordView) << "\": \n\t{\n"
                << "\t\t\"" << JSON_ATTRIBUTE_SEVERITY << "\":\t\t \"" << boost::log::extract<severity_levels>("Severity", recordView) << "\",\n"
                << "\t\t\"" << JSON_ATTRIBUTE_TIMESTAMP << "\":\t \"" << boost::log::extract<boost::log::attributes::utc_clock::value_type>("TimeStamp", recordView) << "\",\n"
                << "\t\t\"" << JSON_ATTRIBUTE_UPTIME << "\":\t\t \"" << boost::log::extract<boost::log::attributes::timer::value_type>("Uptime", recordView) << "\",\n"
                << "\t\t\"" << JSON_ATTRIBUTE_THREAD_ID <<"\":\t \"" << boost::log::extract<boost::log::attributes::current_thread_id::value_type>("ThreadID", recordView) << "\",\n"
                << "\t\t\"" << JSON_ATTRIBUTE_PROCESS_ID << "\":\t \"" << boost::log::extract<boost::log::attributes::current_process_id::value_type>("ProcessID", recordView) << "\",\n"
                << "\t\t\"" << JSON_ATTRIBUTE_PROCESS << "\":\t\t \"" << boost::log::extract<std::string>("Process", recordView) << "\",\n"
                //TODO made this an issue - these two things will always have the value of this file and the line number around here...
                // << "\t\t\"" << JSON_ATTRIBUTE_SOURCE_FILE << "\": \"" << __FILE__ << "\",\n"
                // << "\t\t\"" << JSON_ATTRIBUTE_SOURCE_LINE_NUMBER << "\": \"" << __LINE__ << "\",\n"
                << "\t\t\"" << JSON_ATTRIBUTE_MESSAGE << "\":\t\t \"" << boost::log::extract<std::string>("Message", recordView) << "\"\n"
                << "\t}"
            ;
            
            if(not emptyFile)
            {            
                if(close_out(file) and (_file_count_limit > 1))
                {
                    rotate();
                }
            }        
        }

        void logger::add_reference()
        {
            if(_references <= 0)
            {
                boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::utc_clock());
                boost::log::core::get()->add_global_attribute("Uptime", boost::log::attributes::timer());
                boost::log::core::get()->add_global_attribute("ThreadID", boost::log::attributes::current_thread_id());
                boost::log::core::get()->add_global_attribute("ProcessID", boost::log::attributes::current_process_id());
                boost::log::core::get()->add_global_attribute("Process", boost::log::attributes::current_process_name());
                boost::log::core::get()->add_global_attribute("LineID", boost::log::attributes::counter<size_t>());

                _console_sink = boost::log::add_console_log
                (
                    std::clog, 
                    boost::log::keywords::format = 
                        boost::log::expressions::stream 
                            << _uptime << " " 
                            << _severity << " "
                            << '[' << _threadID << ':' << _processID << ' ' << boost::filesystem::path(__FILE__).filename() << ':' << __LINE__ << "] "
                            << _process << ": "
                            << boost::log::expressions::message
                );
                
                //TODO syslog server -> json     

                file_sink();
                severity_level(_severity_level);
            }        

            //TODO in a different branch
            // if(not _filenames.contains(DEFAULT_LOG_FILENAME))
            // {
            //     _file_sink = boost::log::add_file_log
            //     (
            //         DEFAULT_LOG_FILENAME,
            //         boost::log::keywords::format = &Logger::formatter,
            //         boost::log::keywords::auto_flush = true
            //     );

            //     _filenames.insert(DEFAULT_LOG_FILENAME_FORMAT);
            // }
            
            ++_references;
        }

        logger::logger(const boost::filesystem::path& working_directory)     
        {
            //XXX this won't play nice with multiple references
            _working_directory = working_directory;
            add_reference();
        }

        logger::logger(const std::string filename, const boost::filesystem::path& working_directory)
        {
            _working_directory = working_directory;
            _filename = filename;
            add_reference();
        }

        logger::logger()
        {
            add_reference();
        }

        logger::~logger()
        {        
            if(--_references <= 0)
            {
                auto currentLogFile = _file_sink->locked_backend()->get_current_file_name();
                _current_file_rotation_index = 1;  

                close_out(currentLogFile, false);
                boost::log::core::get()->remove_all_sinks();                      
            }
        }

        size_t logger::references()
        {
            return _references;
        }

        void logger::references(const size_t& value)
        {
            _references = value;
        }

        filepath_vector logger::targets()
        {
            filepath_vector returnVector = { _file_sink->locked_backend()->get_current_file_name() };

            //TODO add support for more than one file logger and convert whatever the structure is to a vector
            return returnVector;
        }

        int logger::get_file_number(const boost::filesystem::path& target)
        {
            int returnValue = -1;
            std::smatch matches;
            std::string asString(target.string());

            if(std::regex_search(asString, matches, FILENUMBER_REGEX))
            {
                try
                {
                    returnValue = std::stoi(matches[0]);
                }
                catch(const std::invalid_argument& exception)
                {
                    LOGGER_UNUSED(exception);
                    //TODO determine if we can exclusively log something to console and put this there
                    //std::cerr << exception.what() << '\n';
                }
            }
            
            return returnValue;
        }

        void logger::rotate()
        {
            auto file = _file_sink->locked_backend()->get_current_file_name();
            auto it = boost::filesystem::directory_iterator(_working_directory);
            size_t encountered = 0;
            bool replaceSmallestFile = false;
            boost::filesystem::path replaceFile;
            int replaceFileNumber;
            size_t replaceIndex = (_current_file_rotation_index >= _file_count_limit) ? 1 : _current_file_rotation_index;
            for(auto&& entry : it)
            {
                if(entry.is_regular_file())
                {
                    auto path = entry.path();

                    if(std::regex_match(path.string(), FILENAME_REGEX))
                    {
                        int fileNumber = get_file_number(path);
                        
                        if(replaceFile.empty())
                        {
                            replaceFile = path;
                            replaceFileNumber = fileNumber;
                        }
                        else
                        {
                            if(fileNumber != INVALID_FILENUMBER)
                            {
                                if(fileNumber == replaceIndex)
                                {
                                    replaceFileNumber = fileNumber;
                                    replaceFile = path;
                                }
                            }
                        }

                        ++encountered;
                    }

                    if(replaceSmallestFile = (encountered >= _file_count_limit))
                    {
                        _current_file_rotation_index = replaceFileNumber;
                        break;
                    }
                }            
            }
            
            close_out(file, false);
            
            if(replaceSmallestFile)
            {
                boost::system::error_code ec;
                boost::filesystem::remove(replaceFile, ec);
                
                if(ec.value() != 0)
                {
                    std::cerr << ec.what() << std::endl;
                }
            }

            file_sink();
        }

        void logger::file_sink()
        {
            if(_file_sink)
            {
                boost::log::core::get()->remove_sink(_file_sink);
                _file_sink.reset();
            }
            
            _file_sink = boost::log::add_file_log
                (
                    construct_file_path(),
                    boost::log::keywords::format = &logger::formatter,
                    boost::log::keywords::auto_flush = true
                );
        }

        boost::filesystem::path logger::construct_file_path()
        {
            //NOTE Make a copy because append modifies in place
            auto tempWorkingDirectory(_working_directory);
            if(_filename.ends_with(".json"))
            {
                _filename.erase(_filename.find_last_of('.'));
            }
            tempWorkingDirectory = tempWorkingDirectory.append(boost::str(DEFAULT_LOG_FILENAME_FORMAT % _filename % _current_file_rotation_index++));

            if(not boost::filesystem::exists(_working_directory))
            {
                //TODO permission checks
                boost::filesystem::create_directory(_working_directory);
            }

            return tempWorkingDirectory;
        }

        bool logger::close_out(const boost::filesystem::path& target, bool sizeCheck)
        {
            std::fstream logFile(target.string(), std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
            bool close = true;

            if(logFile.is_open())
            {
                if(sizeCheck)
                {
                    size_t file_size = logFile.tellg();
                    close = file_size >= _file_size_limit;
                }
                
                if(close)
                {
                    logFile << "}";                    
                }

                logFile.close();   
            }

            return close;
        }

        severity_levels logger::severity_level()
        {
            return _severity_level;
        }

        void logger::severity_level(const severity_levels& severity_level)
        {
            if(_severity_level != severity_level)
            {
                _severity_level = severity_level;
                boost::log::core::get()->set_filter(boost::log::expressions::attr<severity_levels>("Severity") <= severity_level);
            }        
        }

        void logger::emergency(const std::string& message)
        {
            BOOST_LOG_SEV(_logger, severity_levels::emergency) << message;
        }

        void logger::alert(const std::string& message)
        {
            BOOST_LOG_SEV(_logger, severity_levels::alert) << message;
        }

        void logger::critical(const std::string& message)
        {
            BOOST_LOG_SEV(_logger, severity_levels::critical) << message;
        }

        void logger::error(const std::string& message)
        {
            BOOST_LOG_SEV(_logger, severity_levels::error) << message;
        }

        void logger::warning(const std::string& message)
        {
            BOOST_LOG_SEV(_logger, severity_levels::warning) << message;
        }

        void logger::notice(const std::string& message)
        {
            BOOST_LOG_SEV(_logger, severity_levels::notice) << message;
        }

        void logger::informational(const std::string& message)
        {        
            BOOST_LOG_SEV(_logger, severity_levels::informational) << message;
        }

        void logger::debug(const std::string& message)
        {
            BOOST_LOG_SEV(_logger, severity_levels::debug) << message;
        }

        void logger::file_size(const size_t& value)
        {
            _file_size_limit = value;
        }

        size_t logger::file_size()
        {
            return _file_size_limit;
        }

        void logger::working_directory(const boost::filesystem::path& value)
        {
            close_out(_file_sink->locked_backend()->get_current_file_name());
            
            _working_directory = value;
            
            file_sink();
        }
        
        boost::filesystem::path logger::working_directory()
        {
            return _working_directory;
        }

        void logger::maximum_file_count(const size_t& value)
        {
            _file_count_limit = value;
        }
        
        size_t logger::maximum_file_count()
        {
            return _file_count_limit;
        }

        boost::filesystem::path logger::current_log_file()
        {
            return _file_sink->locked_backend()->get_current_file_name();
        }

        void logger::clear_working_directory() noexcept
        {
            try
            {
                rotate();
                boost::filesystem::remove_all(_working_directory);
            }
            catch(const boost::filesystem::filesystem_error& exception)
            {
                std::cerr << "The following exception was thrown while trying to empty the working directory: " << exception.what() << std::endl;
            }
        }
    }    
}