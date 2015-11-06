#include "logger.h"

Logger Logger::instance;

using namespace MNET;

void Logger::init()
{
    if( !inited ){
        JsonConf *conf = JsonConf::getInstance();
        std::string confLevelStr = conf->find("LogFile", "level");
        std::string levelStr = confLevelStr.empty() ? "1" : confLevelStr;
        int level = atoi(levelStr.c_str());
        if(level > 9 || level < 0) level = 1; //debug

        std::string confLogPath = conf->find("LogFile", "path");
        std::string logPath = confLogPath.empty() ? defaultLogPath : confLogPath;

        spdlog::set_level((spdlog::level::level_enum)level);
        fileLogger = spdlog::daily_logger_mt("Daily", logPath + "daily", 0, 0, true );
        consoleLogger = spdlog::stdout_logger_mt("Console");

        inited = true;
    }
}
