#include"httplib.h"
#include"Compile.hpp"
#include"Util.hpp"
#include<json/json.h>
#include<unordered_map>

int main()
{
	//创建server对象
  cout<<"hello server!"<<endl;
  using namespace httplib;
	Server server;
	server.Get("/hi",[](const Request& req,Response& resp){
			(void)req;
			cout<<"get func"<<endl;
      Json::Value req0;
			req0["code"] = "#include <stdio.h>\n int main(){printf(\"hello\");return 0;}";
			req0["stdin"] = "";
			Json::Value resp0;
			Compiler::CompileAndRun(req0,&resp0);
			Json::FastWriter writer;
			LOG(INFO)<<writer.write(resp0)<<std::endl;
			resp.set_content(writer.write(resp0),"text/plain");
			});
	server.Post("/compile",[](const Request& req,Response& resp){
			//根据具体的问题，根据请求，计算出响应结果
	    cout<<"post func"<<endl;
      (void)req;
			unordered_map<string,string> body_kv;
			cout<<req.body<<"this is body!"<<endl;
			Util_Url::ParseBody(req.body,&body_kv);     
			//结合Json对象和http请求调用Compile
			Json::Value req_json; 
			Json::Value resp_json;
			for(auto e:body_kv)
			{
				req_json[e.first] = e.second;
				LOG(INFO)<<e.first<<endl;
			}
			Compiler::CompileAndRun(req_json,&resp_json);
			Json::FastWriter writer;
			LOG(INFO)<<writer.write(resp_json)<<endl;
			resp.set_content(writer.write(resp_json),"text/plain");
			});
	server.set_base_dir("./wwwroot");
	server.listen("0.0.0.0",8080);
	return 0;
}
