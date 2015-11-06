#include "spdlog/spdlog.h"
#include <string.h>
#include "jsonconf.h"

class Logger
{
private:
    const std::string defaultLogPath = "logs/";
    Logger() : inited(false) {}

    static Logger instance;

    std::shared_ptr<spdlog::logger> fileLogger;
    std::shared_ptr<spdlog::logger> consoleLogger;
    
    bool inited;

    static Logger *getInstance()
    {
        instance.init();
        return &instance;
    }

    void init();
    
public:
    static std::shared_ptr<spdlog::logger> file() { return getInstance()->fileLogger; }
    static std::shared_ptr<spdlog::logger> console() { return getInstance()->consoleLogger; }
}; //class Logger
