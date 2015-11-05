#include "jsonconf.h"
#include <iostream>

namespace MNET
{

JsonConf JsonConf::instance;
Thread::RWLock JsonConf::locker("JsonConf::locker");

void JsonConf::reLoad()
{
    struct stat st;
    Thread::RWLock::WRScoped l(locker);

    for( stat(fileName.c_str(), &st); mTime != st.st_mtime; stat( fileName.c_str(), &st) ){
        mTime = st.st_mtime;
        std::ifstream ifs( fileName.c_str() );
        SectionType section;
        SectionMap sMap;
        if( !jsonConfMap.empty() ){
            jsonConfMap.clear();
        }
        
        if( !ifs.is_open() ){
            std::cout << "open error! " << std::endl;
            return;
        }
        Json::Reader reader;
        Json::Value root;
        if( !reader.parse(ifs, root, false) ){
            //Error log
            std::cout << "json file parse error! " << std::endl;
            return;
        }

        for( Json::Value::const_iterator fit = root.begin(); fit != root.end(); ++fit){
            std::string sectionKey = fit.key().asString();
            SectionMap mapTmp;
            for( Json::Value::const_iterator sit = fit->begin(); sit != fit->end(); ++sit ){
                std::string key = sit.key().asString();
                mapTmp[key] = sit->asString();
            }
            if( !mapTmp.empty() ){
                jsonConfMap[sectionKey] = mapTmp;
            }
        }
    }
}

void JsonConf::merge(JsonConf& rhs)
{
    Thread::RWLock::WRScoped l(locker);
    for( JsonConfMap::iterator it = rhs.jsonConfMap.begin(); it != rhs.jsonConfMap.end(); ++it ){
        JsonConfMap::iterator selfIt = jsonConfMap.find(it->first);
        if( selfIt == jsonConfMap.end()){
            jsonConfMap[it->first] = it->second;
        } else {
            SectionMap& sectionMap = it->second;
            SectionMap& selfSectionMap = selfIt->second;
            for( SectionMap::iterator sIt = sectionMap.begin(); sIt != sectionMap.end(); ++sIt){
                selfSectionMap[sIt->first] = sIt->second;
            }
        }
    }
}

JsonConf::ValueType JsonConf::find(const SectionType& section, const KeyType& key)
{
    Thread::RWLock::RDScoped l(locker);
    return jsonConfMap[section][key];
}

JsonConf::ValueType JsonConf::put(const SectionType& section, const KeyType& key, const ValueType& value)
{
    Thread::RWLock::WRScoped l(locker);
    ValueType oldValue = jsonConfMap[section][key];
    jsonConfMap[section][key] = value;
    return oldValue;
}

void JsonConf::getKeys(const SectionType& section, std::vector<KeyType>& keys)
{
    keys.clear();
    Thread::RWLock::RDScoped l(locker);
    SectionMap sMap = jsonConfMap[section];
    for( SectionMap::const_iterator it = sMap.begin(); it != sMap.end(); ++it){
        keys.push_back( it->first );
    }
}

};//namespace MNET
