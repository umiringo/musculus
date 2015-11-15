#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include "json/json.h"
#include <map>

struct{
    int tid;
    int cash;
    int amount;
    int month;
    int fund;
    string plat;
    void dump(){
        std::cout << "tid=" << tid << " cash=" << cash << " amount=" << amount;
        std::cout << " month="<< month << " fund=" << fund << " plat=" << plat << std::endl;
    }
} Info;

int main()
{
    std::cout << "Test json config file..." << std::endl;
    std::ifstream ifs;
    ifs.open("configtest.json");

    Json::Reader reader;
    Json::Value root;
    if( !reader.parse(ifs, root, false)){
        std::cout << "Parse json failed!" << std::endl;
        return -1;
    }
    
    std::map<int, Info*> confMap;
    Json::Value webshop = root["webshop"];
     
    //读取array
    for( int i = 0; i < webshop.size(); ++i){
        Info *t = new Info();
        t->tid = webshop[i]["tid"].toInt();
        t->cash = webshop[i]["cash"].toInt();
        t->amount = webshop[i]["amount"].toInt();
        t->month = webshop[i]["month"].toInt();
        t->fund = webshop[i]["fund"].toInt();
        t->plat = webshop[i]["plat"].toString();
        confMap.insert( std::make_pair(t->tid, t) );
    }

    for(std::map<int,Info*>::iterator it = confMap.begin(); it != confMap.end(); ++it){
        it->second->dump();
    }
    return 0;
}
