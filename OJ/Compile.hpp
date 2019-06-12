#pragma once 
#include<iostream>
#include<string>
#include<atomic>

#include<sys/stat.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/fcntl.h>
#include<fcntl.h>

#include<json/json.h>

#include"Util.hpp"
using namespace std;

//////////////////////////////////
//此模块完成在线编译模块的编译功能
//CompileAndRun函数完成编译和运行功能

class Compiler
{
  public:
    //1.参数和返回值，均为JSON格式
    //json::value类jsoncpp中的核心类，借助这个类可以完成序列化和反序列动作
    //使用方法和map类似，也可使用[]完成属性操作

    //约定六种文件
    //统一将这些文件放在temp_file目录下
    //1.源代码文件
    static string SrcPath(const string& name)
    {
      return "./temp_file/"+name+".cpp";
    }
    //2.标准输入文件
    static string StdinPath(const string& name)
    {
      return "./temp_file/"+name+".stdin";
    } 
    //3.编译错误文件
    static string CompileErrPath(const string& name)
    {
      return "./temp_file/"+name+".compile_error";

    }
    //4.可执行文件
    static string ExePath(const string& name)
    {
      return "./temp_file/"+name+".exe";
    }
    //5.标准输出文件
    static string StdoutPath(const string& name)
    {
      return "./temp_file/"+name+".stdout";
    }
    //6.标准错误文件
    static string StderrPath(const string& name)
    {
      return "./temp_file/"+name+".stderr";
    }


    //编译和运行函数
    static bool CompileAndRun(const Json::Value& req,Json::Value* resp)
    {
      LOG(INFO)<<"Compile open"<<endl;
      //1.根据请求对象，生成源代码和标准输入文件
      if(req["code"].empty())
      {
        (*resp)["error"] = 3;
        (*resp)["reason"] = "code empty";
        LOG(ERROR)<<"code empty"<<endl;
        return false;
      }
      //rep["code"]根据key取出value.value类型
      //Json::Value.这个类通过asString()转换字符串

      //通过该类的文件操作函数将代码写入源代码文件，标准输入写入标准输入文件
      //1.生成文件名
      const string code = req["code"].asString();
      const string stdin = req["stdin"].asString();
      string file_name = WriteTmpFile(code,stdin);

      //2.调用编译
      bool ret = Compile(file_name);
      //错误处理
      if(ret == false)
      {
        //这里的错误并非本程序的错误，而是用户代码自身的错误
        //而对于服务器本身并没有出错

        (*resp)["error"] = 1;
        string reason;
        Util_File::Read(CompileErrPath(file_name),&reason);
        (*resp)["reason"] = reason;
        LOG(INFO)<<"Compile failed!"<<endl;
        return false;
      }
      //3.运行
      int sig = Run(file_name);
      //错误处理，Run子进程退出信息
      if(sig != 0)
      {
        (*resp)["error"] = 2;
        (*resp)["reason"] = "Program exit signo: "+to_string(sig);
        LOG(INFO)<<"program exit by signo:"<<to_string(sig)<<endl;
        return false;
      }
      //4.此时程序运行过程正常，将最终结果返回
      (*resp)["error"] = 0;
      (*resp)["reason"] = "";
      string str_stdout;
      Util_File::Read(StdoutPath(file_name),&str_stdout);
      (*resp)["stdout"] = str_stdout;
      string str_stderr;
      Util_File::Read(StderrPath(file_name),&str_stderr);
      (*resp)["stderr"] = str_stderr;
      LOG(INFO)<<"program"<<file_name<<"Done"<<endl;
      return true;
    }
  private:
    //文件处理函数
    //1.为这次请求分配唯一的名字，通过返回值返回
    //2.将源代码写入源代码文件，标准输入写入标准输入文件
    //分配这次请求的名字形如：tmp_1558859633.2
    //tem_+时间戳+文件编号，文件编号必须唯一，防止同一时刻多个请求
    static string WriteTmpFile(const string& code,const string& str_stdin)
    {
      //atomic_int，原子操作，避免线程安全问题
      static atomic_int id(0);
      ++id;
      string file_name = "tmp_"+to_string(Util_Time::TimeStamp())+"."+to_string(id);
      Util_File::Write(SrcPath(file_name),code);
      Util_File::Write(StdinPath(file_name),str_stdin);
      return file_name;
    }

    //编译函数
    static bool Compile(const string& file_name)
    {
      //1.构造编译指令
      //g++ file_name.cpp -o filename.exe -std=c++11
      char* command[10] = {0};
      char buf[10][50] = {{0}};
      //将二位数组的每一行字符串，赋值给指针数组的每一个元素（指针）
      for(int i = 0;i<10;i++)
        command[i] = buf[i];
      sprintf(command[0],"%s","g++");
      sprintf(command[1],"%s",SrcPath(file_name).c_str());
      sprintf(command[2],"%s","-o");
      sprintf(command[3],"%s",ExePath(file_name).c_str());
      sprintf(command[4],"%s","-std=c++11");
      command[5] = NULL;

      //1.父进程等待子进程
      //2.子进程进行程序替换
      int pid = fork();
      if(pid>0)
      {
        //2.父进程进行进程等待
        waitpid(pid,NULL,0);
      }
      else{
        //3.子进程进行程序替换
        //打开编译错误文件，随时将错误信息输出到该文件
        int fd = open(CompileErrPath(file_name).c_str(),O_WRONLY|O_CREAT,0666);
        if(fd<0)
        {
          LOG(ERROR)<<"open Copiler file error"<<endl;
          exit(1);
        }
        //将标准错误重定向到fd
        dup2(fd,STDERR_FILENO);
        execvp(command[0],command);
        //如果程序替换失败，就直接退出
        exit(0);
      }
      //编译过程基本结束
      //此时通过判断是否有可执行程序，来判断是否编译成功
      
      //判断文件存在与否，可是有stat函数
      struct stat st;
      int ret = stat(ExePath(file_name).c_str(),&st);
      if(ret < 0)
      {
        //ret<0，说明文件不存在
        LOG(INFO)<<"Compile failed!"<<file_name<<endl;
        return false;
      }
      LOG(INFO)<<"Comppiler"<<file_name<<"OK!"<<endl;
      return true;
    }

    //运行
    static int Run(const string& file_name)
    {
      //1.创建子进程，父进程进行进程等待，子进程运行可执行程序
      int ret = fork();
      if(ret>0)
      {
        //2.父进程进行进程等待
        int status = 0;
        waitpid(ret,&status,0);
        //最后7位为信号
        return status & 0x7f;
      }
      else 
      {
        //3.子进程进行进程替换，替换之前将标准输入，标准输出、标准错误重定向
        int fd_stdin = open(StdinPath(file_name).c_str(),O_RDONLY);
        
        int fd_stdout = open(StdoutPath(file_name).c_str(),O_WRONLY|O_CREAT,0666);
        int fd_stderr = open(StderrPath(file_name).c_str(),O_WRONLY|O_CREAT,0666);
      dup2(fd_stdin,0);
      dup2(fd_stdout,1);
      dup2(fd_stderr,2);
      //4.子进程进行程序替换
      execl(ExePath(file_name).c_str(),ExePath(file_name).c_str(),NULL);
      exit(0);
      }
    }
};
