#include "spdlog/spdlog.h"
#include <string.h>
#include "jsonconf.h"

class SpdLogger
{
    
private:
    const std::string defaultLogPath = "logs/";

    SpdLogger() : inited(false) {}

    static SpdLogger instance;
    
    std::shared_ptr<spdlog::logger> debugLogger; //debug
    std::shared_ptr<spdlog::logger> infoLogger; //info
    std::shared_ptr<spdlog::logger> verboseLogger; //trace
    std::shared_ptr<spdlog::logger> warnLogger; //warn
    std::shared_ptr<spdlog::logger> errorLogger; //err
    std::shared_ptr<spdlog::logger> consoleLogger; //debug
    
    std::string logPath;
    std::string todayLogPath;
    struct tm logDate;

    bool inited;

    static SpdLogger *Logger()
    {
        instance.init();
        return &instance;
    }

    void init();
    void initLogger(std::string& path);
public:
    static std::shared_ptr<spdlog::logger> debug() { return Logger()->debugLogger; }
    static std::shared_ptr<spdlog::logger> info() { return Logger()->infoLogger; }
    static std::shared_ptr<spdlog::logger> verbose() { return Logger()->verboseLogger; }
    static std::shared_ptr<spdlog::logger> warn() { return Logger()->warnLogger; }
    static std::shared_ptr<spdlog::logger> error() { return Logger()->errorLogger; }
    static std::shared_ptr<spdlog::logger> console() { return Logger()->consoleLogger; }
    
};//class Logger

