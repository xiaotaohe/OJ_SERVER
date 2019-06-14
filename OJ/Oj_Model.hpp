#pragma once 
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<algorithm>
#include"Util.hpp"
using namespace std;
//model模块负责题目管理

//题目的组织形式
struct Question{
  //1.题目列表视图部分
  string id;//题目ID
  string name;//题目名称
  string dir;//题目对应的目录
  string star;//题目难度

  //2.单个题目的视图部分
  string desc;//题目的描述
  string header_cpp;//题目的代码框架
  string tail_cpp;//题目测试用例代码
};

class Oj_Model{
  private:
    map<string,Question> model_;
  public:
    //1.加载OJ_Config.cfg文件
    bool Load()
    {
      //1.先打开OJ_Config.cfg文件
      ifstream file("./OJ_Data/OJ_Config.cfg");
      if(!file.is_open())
        return false;
      //2.按行读取题目，每行为一题
      string line;
      while(getline(file,line))
      {
        //3.解析每行，拼装Questions结构体
        vector<string> tokens;
        tokens.clear();
        Util_String::Split(line,"\t",&tokens);
        if(tokens.size() != 4)
        {
          LOG(ERROR)<<"config file format error"<<endl;
          continue;
        }
        //4.瓶装结构体，并加入map表
        Question q;
        q.id = tokens[0];
        q.name = tokens[1];
        q.star = tokens[2];
        q.dir = tokens[3];
        cout<<q.id<<endl;
        Util_File::Read(q.dir+"/desc.txt",&q.desc);
        Util_File::Read(q.dir+"/header.cc",&q.header_cpp);
        Util_File::Read(q.dir+"/tail.cc",&q.tail_cpp);
        //5.放入map表
        model_[q.id] = q;
      }
      file.close();
      LOG(INFO)<<"Load"<<": "<<model_.size()<<" "<<"questions"<<endl;
      return true;
    }
  
    //2.获取所有题目
    bool GetAllQuestions(vector<Question>* question) const{
      //遍历map表即可
      question->clear();
      for(const auto& e : model_)
        question->push_back(e.second);
      return false;
    }
    
    //3.获取单个题目
    bool GetQuestion(const string id,Question* q) const{
      map<string,Question>::const_iterator it = model_.find(id);
      if(it == model_.end()){
        LOG(INFO)<<"未找到题目"<<endl;
        return false;
      }
      *q = it->second;
      return true;
  }
};
