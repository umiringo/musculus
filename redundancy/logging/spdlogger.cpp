#include "spdlogger.h"
#include <unistd.h>

SpdLogger SpdLogger::instance;

using namespace MNET;

void SpdLogger::initLogger(std::string& path)
{
    spdlog::drop_all();

    debugLogger = spdlog::rotating_logger_mt( "Debug", path + "debug", 10485760, 3 );
    /* debugLogger->set_level(spdlog::level::debug); */

    infoLogger = spdlog::rotating_logger_mt( "Info", path + "info", 10485760, 3 );
    /* infoLogger->set_level(spdlog::level::info); */

    verboseLogger = spdlog::rotating_logger_mt( "Verbose", path + "verbose", 10485760, 3 );
    /* verboseLogger->set_level(spdlog::level::trace); */

    warnLogger = spdlog::rotating_logger_mt( "Warning", path + "warn", 10485760, 3);
    /* warnLogger->set_level(spdlog::level::warn); */

    errorLogger = spdlog::rotating_logger_mt( "Error", path + "error", 10485760, 3);
    /* errorLogger->set_level(spdlog::level::err); */

    consoleLogger = spdlog::stdout_logger_mt("Console");
    /* consoleLogger->set_level(spdlog::level::debug); */
}

void SpdLogger::init()
{
    time_t now = time(NULL);
    if(!inited){
        JsonConf *conf = JsonConf::getInstance();

        std::string confLogPath = conf->find("LogFile", "path");
        logPath = confLogPath.empty() ? defaultLogPath : confLogPath;
        logDate = *localtime( &now );
        char buf[64];
        strftime( buf, 64, "%Y-%m-%d", &logDate);
        std::string dateStr( buf );
        dateStr += "/";
        todayLogPath = logPath + dateStr;

        if(access(todayLogPath.c_str(), 0) == -1){
            mkdir(todayLogPath.c_str(), 0777);
        }

        initLogger(todayLogPath); 
        inited = true;
    } else {
        tm curDate = *localtime( &now );
        if(curDate.tm_year == logDate.tm_year &&
            curDate.tm_mon == logDate.tm_mon &&
            curDate.tm_mday == logDate.tm_mday) {
            return;
        }
        std::cout << "init again!"  << std::endl;
        logDate = curDate;
        char buf[64];
        strftime( buf, 64, "%Y-%m-%d", &logDate);
        std::string dateStr( buf );
        dateStr += "/";
        todayLogPath = logPath + dateStr;
        
        if(access(todayLogPath.c_str(), 0) == -1){
            mkdir(todayLogPath.c_str(), 0777);
        }

        initLogger(todayLogPath);
    }
}

