#pragma once
#include<unistd.h>
#include<stdio.h>
#include<cstdlib>
#include<fstream>
#include<iostream>
#include<sys/time.h>
#include<sys/types.h>
#include<string>
//第三方库boost库
#include<boost/algorithm/string.hpp>
#include<unordered_map>
using namespace std;

//获取时间戳
class Util_Time
{
  public:
    static int64_t TimeStamp()
    {
      //秒级
      struct timeval tv;
      gettimeofday(&tv,NULL);
      return tv.tv_sec;
    }
    static int64_t TimeStampMS()
    {
      //毫秒级
      struct timeval tv;
      gettimeofday(&tv,NULL);
      return tv.tv_sec*1000+tv.tv_usec/1000;
    }
};

//打印日志
//日志级别：
//FATAL错误
//ERROR致命错误
//WARNING警告
//INFO提示
//日志形式：
//[F1558771168 Util.hpp:32] hello
//[E1558771168 Util.hpp:32] hello
//[W1558771168 Util.hpp:32] hello
//[I1558771168 Util.hpp:32] hello

//日志级别 enum枚举类型
enum Level
{
  INFO,//提示
  WARNING,//警告
  ERROR,//错误
  FATAL//致命错误
};

inline ostream& Log(Level level,const string& filename,int line_num)
{
  std:: string prefix = "[";
  if(level == INFO)
    prefix += "I";
  if(level == WARNING)
    prefix += "W";
  if(level == ERROR)
    prefix += "E";
  if(level == FATAL)
    prefix += "F";
  prefix += to_string(Util_Time::TimeStamp());
  prefix += " ";
  prefix +=filename;
  prefix += ": ";
  prefix += to_string(line_num);
  prefix += "] ";
  cout<<prefix;
  return cout;
}
#define LOG(level) Log(level,__FILE__,__LINE__)


//2.文件工具类
class Util_File
{
  public:
    //传入文件名对文件操作，传入/传出要写/读的内容
    static bool Read(const string& filepath,string* content)
    {
      content->clear();
      ifstream file(filepath.c_str());
      if(!file.is_open())
        return false;
      //1.按行读取，使用getline接口
      //2.注意getline会默认去掉一行内容的"\n"
      string line;
      while(getline(file,line))
        *content += line+"\n";
      file.close();
      return true;
    }
    static bool Write(const string& filepath,const string& content)
    {
      ofstream file(filepath.c_str());
      if(!file.is_open())
        return false;
      cout<<"Write: "<<content<<endl;
      //写操作，用追加好还是write函数好呢
      file<<content;
      file.close();
      return true;
    }
};


//3.字符串切分
//采用第三放库boost库 split函数
class Util_String
{
  public:
    static void Split(const string& input,const string& split_char,vector<string>* output)
    {
      boost::split(*output,input,boost::is_any_of(split_char),boost::token_compress_off);
    }
};

//4.Url解析
class Util_Url
{
  public:
    static void ParseBody(const string& body,unordered_map<string,string>* params){
      //1.先对body进行字符串切分
      //body中每个字段以&符为分解
      vector<string> kvs;
      cout<<"解析body\n";
      Util_String::Split(body,"&",&kvs);
      //2.对解析出来的字段进行解析
      //字段例如：code = "include....."
      for(size_t i = 0;i<kvs.size();i++)
      {
        vector<string> kv;
        Util_String::Split(kvs[i],"=",&kv);
        //每个字段可以解析出两部分，如果不为两部分则出错
        if(kv.size() != 2)
          continue;
        //对于这两部分是之前约定的：code、stdin
        //对于key:"code"、"stdin"没有特殊字符不用urldecode
        //而value需要解码
        (*params)[kv[0]] = UrlDecode(kv[1]); 
      }
    }

  private:
    
    unsigned char ToHex(unsigned char x) 
    { 
      return  x > 9 ? x + 55 : x + 48; 
    }

    static unsigned char FromHex(unsigned char x) 
    { 
      unsigned char y;
      if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
      else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
      else if (x >= '0' && x <= '9') y = x - '0';
      else assert(0);
      return y;
    }

    static std::string UrlDecode(const std::string& str)
    {
      std::string strTemp = "";
      size_t length = str.length();
      for (size_t i = 0; i < length; i++)
      {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%')
        {
          assert(i + 2 < length);
          unsigned char high = FromHex((unsigned char)str[++i]);
          unsigned char low = FromHex((unsigned char)str[++i]);
          strTemp += high*16 + low;

        }
        else strTemp += str[i];
      }
      return strTemp;
    }

};

/////////////////////////////////////////////////////////////
//测试工具类
void test()
{
  //测试时间戳和日志
  LOG(ERROR);
  //测试文件操作
  string buf;
  Util_File::Read("test_util_temp",&buf);
  string str = "hfuygyufgyurugurguu";
  Util_File::Write("test_util_temp",str);
}


