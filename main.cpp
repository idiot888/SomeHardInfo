#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>



#define FLOAT_ZERO 0.000001

std::string g_saveFileName;

bool splitString(const std::string& inputStr,std::vector<std::string>& result,const std::string& splitSymbol)
{ 
  
  using namespace std;
  bool ret;

  string::size_type pos1, pos2;
  pos2 = inputStr.find(splitSymbol);
  pos1 = 0;
  while(string::npos != pos2)
  {
     result.push_back(inputStr.substr(pos1, pos2-pos1));
     pos1 = pos2 + splitSymbol.size();
     pos2 = inputStr.find(splitSymbol, pos1);
  }
  if(pos1 != inputStr.length())
    result.push_back(inputStr.substr(pos1));

  return ret;
}
bool trimString(std::string & str)
{       
	using namespace std;
	bool ret;
	int pos1,pos2;
	int pos;
	if( !str.empty() )
        {
	    str.erase(0,str.find_first_not_of(" "));
	    pos1 = str.find_last_not_of(" ");
	    pos2 = str.find_last_not_of("	");
            pos = (pos1 < pos2)?pos1:pos2;
	    str.erase(pos + 1);
	}
	return ret;
}
struct NCpuInfo
{
   std::string realCount;
   std::string coreCount;
   std::string frequency;
   std::string name;
};

struct NMemInfo
{
	std::string totalSize;
	double usage;
};

struct NDiskInfo
{
	int totalSize;
};

int getCpuInfo(NCpuInfo & info)
{       
        using namespace std;
	string fileName = "/proc/cpuinfo";
	string lineContent;
	ifstream input(fileName.c_str());
	int coreArray[32]={0};
	//cout<<t<<"###"<<endl;
	while(getline(input,lineContent))
	{
                vector<string> v;
		splitString(lineContent,v,string(":"));
		if(v.size() != 2)
		{
			continue;
		}
                string item = v[0];
	        trimString(item);
		string value = v[1];
	        trimString(value);
                if(item == "cpu MHz")
		{
			info.frequency = value;
		}
		else if(item == "model name")
		{
			info.name = value;
		}
		else if(item == "cpu cores")
		{
                        info.coreCount = value;
		}
		else if(item == "physical id")
		{
			int index = atoi(value.c_str());


			coreArray[index]++;
		}
	}
	int realCount = 0;
	for(int i=0;i<32;i++)
	{
                if(coreArray[i]>0){realCount++;}
	}
        char strCount[8]={0};
        sprintf(strCount,"%d",realCount);
	info.realCount = string(strCount);
	return 0;
}
int getMemInfo(NMemInfo & info)
{
        using namespace std;
	string fileName = "/proc/meminfo";
	string lineContent;
	ifstream input(fileName.c_str());
	string memFree;
	string memBuffer;
	string memCached;
	while(getline(input,lineContent))
	{
                vector<string> v;
		splitString(lineContent,v,string(":"));
                string item = v[0];
		trimString(item);
		if( item ==  "MemTotal")
		{
                	string value = v[1];
			trimString(value);
                        value.erase(value.size()-2);
                        info.totalSize = value;
		}
		else if( item == "MemFree")
		{       
			memFree = v[1];
                        string & value = memFree;
			trimString(value);
                        value.erase(value.size()-2);


		}
		else if( item == "Buffers")
		{       
 	                memBuffer = v[1];
                        string & value = memBuffer;
			trimString(value);
                        value.erase(value.size()-2);

		}
		else if( item == "Cached")
		{
                        memCached = v[1];
                        string & value = memCached;
			trimString(value);
                        value.erase(value.size()-2);

		}
		else
		{
			break;
		}
	}
	long total = atol(info.totalSize.c_str());
        long free =  atol(memFree.c_str());
	long buffer = atol(memBuffer.c_str());
	long cached = atol(memCached.c_str());
	double usagePercent =1- ((double)(free+buffer+cached)/total);
	usagePercent = usagePercent * 100;
	usagePercent = round(usagePercent *100)/100;
        info.usage = usagePercent;
	
	return 0;

}
int getDiskInfo(NDiskInfo & info)
{
	struct statfs disk_info;
	char path[] = "/";
	int ret = 0;
	if( ret == statfs(path,&disk_info) == -1)
	{
		printf("1111111111111\n");
		return -1;
	}
	long long total_size = disk_info.f_blocks * disk_info.f_bsize;
	info.totalSize = total_size >> 30;
	 
}
int getCpuUsage(double& cpuUsage )
{
   FILE *fstream = NULL;
   char buff[1024]={0};
   char command[]="top -n1 | awk -v n=9 'NR>7{print $(n+1)}'";
   if(NULL == (fstream = popen(command,"r")))      
   {     
           //fprintf(stderr,"execute command failed: %s",strerror(errno));
	   printf("%d errno\n",errno);
           return -1;      
   }

   double dd = 0.0;
   while(NULL != fgets(buff, sizeof(buff), fstream)) 
   { 
       char * pos = strchr(buff,'\n');
       if(pos != NULL)
       {
	       *pos = 0;
       }
       
       double tmp = atof(buff);
       if( tmp > -FLOAT_ZERO && tmp < FLOAT_ZERO)
       {
	       break;
       }
       else
       {
               dd += tmp;
       }
   }
   //std::cout<<"dd "<<dd <<std::endl;
   cpuUsage = dd;
   //printf("cpuuseage %f \n",cpuUsage);
   pclose(fstream);   
}
int getMemoryUsage(double& memoryUsage )
{
   FILE *fstream = NULL;
   char buff[1024]={0};
   char command[]="top -n1 | awk -v n=10 'NR>7{print $(n+1)}'";
   if(NULL == (fstream = popen(command,"r")))      
   {     
           //fprintf(stderr,"execute command failed: %s",strerror(errno));
	   printf("%d errno\n",errno);
           return -1;      
   }

   double dd = 0.0;
   while(NULL != fgets(buff, sizeof(buff), fstream)) 
   { 
       char * pos = strchr(buff,'\n');
       if(pos != NULL)
       {
	       *pos = 0;
       }
       
       double tmp = atof(buff);
       if( tmp > -FLOAT_ZERO && tmp < FLOAT_ZERO)
       {
	       break;
       }
       else
       {
               dd += tmp;
       }
   }
   memoryUsage = dd;   
   pclose(fstream);   
}

void realFunc(int pramter)
{
	//printf("call realFunc \n");
	using namespace std;
	int ret = 0;
	NCpuInfo info;
	NMemInfo info2;
	NDiskInfo info3;
        getCpuInfo(info);
	getMemInfo(info2);
	getDiskInfo(info3);
	double dvalue = atof(info2.totalSize.c_str());
	dvalue = dvalue/(1000 * 1000);
	dvalue = round(dvalue *100)/100;

	double dfrequency = atof(info.frequency.c_str());
        dfrequency = dfrequency/1000;
	dfrequency = round(dfrequency*100)/100;
        double cpuUsage;
        getCpuUsage(cpuUsage);
        
	int realCount = atoi(info.realCount.c_str());
	int coreCount = atoi(info.coreCount.c_str());
        double cpuUsageReal = cpuUsage/(realCount * coreCount);
        double memoryUseage;

	//getMemoryUsage(memoryUseage);

        /*
        fstream out("info.txt",ios::out | ios::trunc);
        out<<"cpu 个数 : "<<info.realCount<<endl;
        out<<"cpu 核数 : "<<info.coreCount<<endl;
        out<<"cpu 主频 : "<<dfrequency <<"GHz"<<endl;
        out<<"cpu 名称 : "<<info.name<<endl;
        out<<"内存大小 : "<<dvalue<<"GB"<<endl;
	out<<"硬盘大小 : "<<info3.totalSize<<"GB"<<endl;
	out<<"cpu使用率 : "<<cpuUsageReal<<"%"<<endl;
	out<<"内存使用率 : "<<memoryUseage<<"%"<<endl;
         
	out.close();
	*/
        fstream out(g_saveFileName.c_str(),ios::out | ios::trunc);
        out<<info.realCount<<endl;
        out<<info.coreCount<<endl;
        out<<dfrequency <<"GHz"<<endl;
        out<<dvalue<<"GB"<<endl;
	out<<info3.totalSize<<"GB"<<endl;
	out<<cpuUsageReal<<"%"<<endl;
	out<<info2.usage<<"%"<<endl;
	out.close();

	/*
        cout<<"cpu 个数 : "<<info.realCount<<endl;
        cout<<"cpu 核数 : "<<info.coreCount<<endl;
        cout<<"cpu 主频 : "<<dfrequency <<"GHz"<<endl;
        cout<<"cpu 名称 : "<<info.name<<endl;
        cout<<"内存大小 : "<<dvalue<<"GB"<<endl;
	cout<<"硬盘大小 : "<<info3.totalSize<<"GB"<<endl;
	cout<<"cpu使用率 : "<<cpuUsageReal<<"%"<<endl;
	cout<<"内存使用率 : "<<memoryUseage<<"%"<<endl;
	*/
        /*
        cout<<info.realCount<<endl;
        cout<<info.coreCount<<endl;
        cout<<dfrequency <<"GHz"<<endl;
        cout<<dvalue<<"GB"<<endl;
	cout<<info3.totalSize<<"GB"<<endl;
	cout<<cpuUsageReal<<"%"<<endl;
	cout<<memoryUseage<<"%"<<endl;
	*/
	//return ret;

}
int main(int argc,char* argv[])
{       
	/*
	using namespace std;
	int ret = 0;
	NCpuInfo info;
	NMemInfo info2;
	NDiskInfo info3;
        getCpuInfo(info);
	getMemInfo(info2);
	getDiskInfo(info3);
	double dvalue = atof(info2.totalSize.c_str());
	dvalue = dvalue/(1000 * 1000);
	dvalue = round(dvalue *100)/100;

	double dfrequency = atof(info.frequency.c_str());
        dfrequency = dfrequency/1000;
	dfrequency = round(dfrequency*100)/100;
        double cpuUsage;
        getCpuUsage(cpuUsage);
        
	int realCount = atoi(info.realCount.c_str());
	int coreCount = atoi(info.coreCount.c_str());
        double cpuUsageReal = cpuUsage/(realCount * coreCount);
        double memoryUseage;

	getMemoryUsage(memoryUseage);

        fstream out("info.txt",ios::out | ios::trunc);
        out<<"cpu 个数 : "<<info.realCount<<endl;
        out<<"cpu 核数 : "<<info.coreCount<<endl;
        out<<"cpu 主频 : "<<dfrequency <<"GHz"<<endl;
        out<<"cpu 名称 : "<<info.name<<endl;
        out<<"内存大小 : "<<dvalue<<"GB"<<endl;
	out<<"硬盘大小 : "<<info3.totalSize<<"GB"<<endl;
	out<<"cpu使用率 : "<<cpuUsageReal<<"%"<<endl;
	out<<"内存使用率 : "<<memoryUseage<<"%"<<endl;
         
	out.close();
	
        cout<<"cpu 个数 : "<<info.realCount<<endl;
        cout<<"cpu 核数 : "<<info.coreCount<<endl;
        cout<<"cpu 主频 : "<<dfrequency <<"GHz"<<endl;
        cout<<"cpu 名称 : "<<info.name<<endl;
        cout<<"内存大小 : "<<dvalue<<"GB"<<endl;
	cout<<"硬盘大小 : "<<info3.totalSize<<"GB"<<endl;
	cout<<"cpu使用率 : "<<cpuUsageReal<<"%"<<endl;
	cout<<"内存使用率 : "<<memoryUseage<<"%"<<endl;
	*/
        if(argc >= 2)
	{
		g_saveFileName = argv[1];
	}
	else
	{
		g_saveFileName = "info.txt";
	}

	int ret;
	signal(SIGALRM,realFunc);
        struct itimerval tick;
        memset(&tick, 0, sizeof(tick));
        tick.it_value.tv_sec = 5;
	tick.it_value.tv_usec = 0;

	tick.it_interval.tv_sec = 5;
	tick.it_interval.tv_usec = 0;

        ret = setitimer(ITIMER_REAL, &tick, NULL);
        if(ret)
	{
		printf("set timer failed \n");
	}
        while(1)
	{
		pause();
	}

	return ret;
}
