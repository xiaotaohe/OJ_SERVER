#pragma once 

#include"Oj_Model.hpp"
#include<ctemplate/template.h>
#include<string>
#include<vector>

class OJ_view{
	//根据数据，生成html这个动作，通常叫网页渲染(reader)
  public:
  static void ReaderAllQuestions(const vector<Question>& all_questions,string* html)
	{
		//将所有的题目数据转换成为题目列表
		//通过网页模板的方式解决
		//流程：
		//1.创建ctemplate对象
		//2.循环的往这个对象中添加一些子对象
		//3.每个子对象再设置一些键值对
		//4.进行数据替换，生成最终的html
    ctemplate::TemplateDictionary dict("all_questions");
		for(const auto& e : all_questions)
		{
			ctemplate::TemplateDictionary* table_dict = dict.AddSectionDictionary("question");
			table_dict->SetValue("id",e.id);
			table_dict->SetValue("name",e.name);
			table_dict->SetValue("star",e.star);
		}

		ctemplate::Template* tpl;
		tpl = ctemplate::Template::GetTemplate("template/all_questions.html",ctemplate::DO_NOT_STRIP);
		tpl->Expand(html,&dict);
	}
	static void RenderQuestion(const Question& question,std::string* html)
	{
		ctemplate::TemplateDictionary dict("questions");
		dict.SetValue("id",question.id);
		dict.SetValue("name",question.name);
		dict.SetValue("star",question.star);
		dict.SetValue("desc",question.desc);
		dict.SetValue("header",question.header_cpp);
		ctemplate::Template* tpl;
		tpl = ctemplate::Template::GetTemplate(
				"template/question.html",
				ctemplate::DO_NOT_STRIP);
		tpl->Expand(html,&dict);
	}
	static void ReaderResult(const string& str_stdout,const string& reason,string* html)
	{
		ctemplate::TemplateDictionary dict("result");
		dict.SetValue("stdout",str_stdout);
		dict.SetValue("reason",reason);
		ctemplate::Template* tpl;
		tpl = ctemplate::Template::GetTemplate("./template/result.html",ctemplate::DO_NOT_STRIP);
		tpl->Expand(html,&dict);
	}
};
