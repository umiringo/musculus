#ifndef __JSONCONF_H__
#define __JSONCONF_H__

#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <string.h>
#include <fstream>
#include "json/json.h"

#include "threadpool.h"

namespace MNET
{

class JsonConf
{
//基于json文件的配置数据管理类，支持两层的json文件
public:
    typedef std::string SectionType;
    typedef std::string KeyType;
    typedef std::string ValueType;

private:
    class StringCaseCmp
    {
    public:
        bool operator() (const std::string& x, const std::string& y) const
        {
            return strcasecmp(x.c_str(), y.c_str()) < 0;
        }
    };

    typedef std::map<KeyType, ValueType, StringCaseCmp> SectionMap;
    typedef std::map<SectionType, SectionMap, StringCaseCmp> JsonConfMap;

    JsonConfMap jsonConfMap;
    std::string fileName;
    time_t mTime;
    static Thread::RWLock locker;
    static JsonConf instance;

    JsonConf():mTime(0) {}

    void reLoad();
    void merge(JsonConf& rhs);

public:
    ValueType find(const SectionType& section, const KeyType& key);
    ValueType put(const SectionType& section, const KeyType& key, const ValueType& value);
    void getKeys(const SectionType& section, std::vector<KeyType>& keys);

    static JsonConf* getInstance(const char *file = NULL)
    {
        if(file && access(file, R_OK) == 0)
        {
            instance.fileName = file;
            instance.mTime = 0;
            instance.reLoad();
        }
        return &instance;
    }
    
    static void AppendConfFile( const char * file )
    {
        JsonConf tmp;
        tmp.fileName = file;
        tmp.mTime = 0;
        tmp.reLoad();
        getInstance()->merge(tmp);
    }

    //来一个直接从文件里面挖数据的函数，只能2层
    static Json::Value digConfFromFile(const char *file, const SectionType& section, const KeyType& key, Json::Value& v)
    {
        std::ifstream ifs;
        ifs.open(file);
        Json::Reader reader;
        Json::Value root;
        if( !reader.parse(ifs, root, false) ){
            return false; 
        }
        if(!root.isMember(section)){
            return false;
        }
        Json::Value kv = root[section];
        if(!kv.isMember(key)){
            return false;
        }
        
        v = kv[key];
        return true;
    }
};//class JsonConf

};//namespace GNET

#endif //__JSONCONF_H__
