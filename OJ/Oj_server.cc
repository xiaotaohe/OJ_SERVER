#include"Util.hpp"
#include"Compile.hpp"
#include"httplib.h"
#include"Oj_Model.hpp"
#include"Oj_view.hpp"

//controller作为服务器核心业务逻辑，需要创建号对应的服务器框架代码
//在这个框架中来组织逻辑

int main()
{
  //服务器启动只加载一次即可
  Oj_Model model;
  model.Load();
  using namespace httplib;
  Server server;
  server.Get("/all_questions",[&model](const Request& req,Response& resp){
      //数据来自Model对象
      (void) req;
      std::vector<Question> all_question;
      model.GetAllQuestions(&all_question);
      //如何借助all_question数据得到最终的html
      std::string html;
      OJ_view::ReaderAllQuestions(all_question,&html);
      resp.set_content(html,"text/html");
  });
  //R"()"c++ 11 引入的语法，原始字符串->忽略字符串中的转义字符
  // \d+ 正则表达式
  server.Get(R"(/question/(\d+))",[&model](const Request& req,Response& resp){
      //LOG(INFO)<<req.matches[0].str()<<","<<req.matches[1].str()<<std::endl;
      Question question;
      model.GetQuestion(req.matches[1].str(),&question);
      std::string html;
      OJ_view::RenderQuestion(question,&html);
      resp.set_content(html,"text/html");
      });
  server.Post(R"(/compile/(\d+))",[&model](const Request& req,Response& resp){
      //此处的实现的代码和compile的实现是很相似的
      //1.先根据id获取道题目的信息
      Question question;
      model.GetQuestion(req.matches[1].str(),&question);
      //2.解析body，获取到用户提交的代码
      std::unordered_map<std::string,std::string> body_kv;//使用unordered_map来表示键值对
      Util_Url::ParseBody(req.body,&body_kv);//请求内容放在body中（post方法），此时需要将内容解析出来
      const std::string& user_code = body_kv["code"];
      //3.构造JSON结构的参数
      Json::Value req_json;//从req对象中获取
      //真实需要编译的代码，是用户提交的代码+题目测试用例的代码
      req_json["code"] = user_code+question.tail_cpp;
      Json::Value resp_json;//resp_json放到响应中
      //4.调用编译模块进行编译
      Compiler::CompileAndRun(req_json,&resp_json);
      Json::FastWriter write;
     //5.根据编译结果构造最终的网页
      std::string html;
      OJ_view::ReaderResult(resp_json["stdout"].asString(),resp_json["reason"].asString(),&html);
      resp.set_content(html,"text/html");
      });
  server.set_base_dir("./wwwroot");
  server.listen("0.0.0.0",9090);
  return 0;
}
