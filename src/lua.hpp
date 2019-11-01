#pragma once

#include <functional>
#include <string>
#include <vector>
#include <map>

class Variable
{
private:

	enum variableType
	{
		lua_number,
		lua_string,
		lua_nil,
		lua_function,
		lua_Cfunction,
		lua_userdata,
		lua_table
	};

	variableType type;

	union lua_values
	{
		void *nil;
		long double number;
		std::string string;
		// std::function<std::string(std::string)> function;
		// Cfunction
		// userdata
		// table

		lua_values()
			: nil(nullptr)
		{}

		lua_values(std::string text)
			: string(text)
		{}

		~lua_values()
		{}
	};

public:
	lua_values value;

public:
	Variable();
	Variable(const Variable &other);
	Variable(std::string text);

	void Equals(std::string var);

	friend std::ostream &operator<<(std::ostream &out, const Variable &var);

public:
	static Variable nil;
};

class Scope
{
public:
	Scope *parent;
	std::vector<std::string> return_value;

private:
	std::map<std::string, Variable> variables;

public:
	Scope();

	Variable &GetVariable(std::string variable_name);
	void SetVariable(std::string vraiable_name, std::string value);
};

class Lua
{
private:
	Scope *global_scope, *current_scope;
	std::istream *code;
	std::string BUFFER;
	bool end;

private:
	void NextWord();
	void NextToken();
	void PutBack();

public:
	Lua();
	~Lua();

	void OpenFile(std::string file_name);
	void ParseStream(std::streambuf *code_ptr);

	void						ParseBlock();
	std::string					ParseVar();
	std::vector<std::string>	ParseVarList();
	std::string					ParseExp();
	std::vector<std::string>	ParseExpList();
	void						ParseStat();
	void						ParseRet();

	Lua &Start();
	Lua &Assign(std::string var, std::string value);
	Lua &Call(std::string function, const std::vector<std::string> &parameters);
	Lua &End();
};
