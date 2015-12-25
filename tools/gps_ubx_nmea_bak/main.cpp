#include <iostream>
#include "Logger.h"
using namespace FrameWork;
int main(int argc,char* argv[]) {
    int iTest = 1000;
    char* pcTest = "log test";
    
	InitLogging(argv[0],INFO,"./log/test");

    //InitLogging(argv[0], WARN ,"./log/test");

    cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	LOG(INFO)<<"info test";
	LOG(WARN)<<"WARN TEST "<<20;
	LOG(ERROR)<<"Error test "<<20<<"nihao";

    LOG(DEBUG)<<"Error test:"<<iTest<<" "<<pcTest;
    LOG(FATAL)<<"Error test:"<<iTest<<" "<<pcTest;

    ostream(0) << "12345600000" << endl;

	Logger::GetInstance().Error("error test common");
	Logger::GetInstance().Fatal("fatal test common %d ",100);
	Logger::GetInstance().Info("info test normal %d %s ",50,"zhongguoren");
	return 0;
}
