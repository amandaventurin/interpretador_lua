#pragma once

#include <functional>
#include <vector>
#include <string>

class Variable
{
public:
	enum variableType
	{
		lua_number,
		lua_string,
		lua_nil,
		lua_function,
		lua_Cfunction,
		// lua_userdata,
		lua_table
	};

private:

	union lua_values
	{
		void *nil;
		long double number;
		std::string string;
		std::function<void()> *function;
		std::function<void()> *Cfunction;
		// userdata
		// table

		lua_values()
			: nil(nullptr)
		{}

		lua_values(std::string text)
			: string(text)
		{}

		lua_values(double number)
			: number(number)
		{}

		~lua_values()
		{}
	};

public:
	variableType type;
	lua_values value;

public:
	Variable();
	Variable(const Variable &other);
	Variable(std::string text);
	Variable(double number);

	void Equals(const Variable &var);
	Variable *Concat(const Variable &var);
	void operator=(const Variable &var);
	void operator()(void);

public:
	static Variable nil;
	friend std::ostream &operator<<(std::ostream &out, const Variable &var);
};

class VariablesList
{
private:
	struct ListElement
	{
		std::string name;
		Variable *value;
		ListElement *next;

		ListElement(std::string name);
		~ListElement();
	};

	ListElement *head, *last;

public:
	VariablesList();
	~VariablesList();

	Variable *Find(std::string name);
	Variable &operator[](std::string name);
	Variable &operator[](int index);
	void PushBack(Variable *var);
	void Reset();
	int Size();

public:
	friend std::ostream &operator<<(std::ostream &out, const VariablesList &list);
};

class Scope
{
public:
	Scope *parent;
	VariablesList return_value;

private:
	VariablesList variables;

public:
	Scope();

	Variable &GetVariable(std::string variable_name);
	void SetVariable(std::string variable_name, const Variable &value);
};

class Lua
{
private:
	Scope *global_scope, *current_scope;
	std::istream *code;
	std::string BUFFER;
	VariablesList EXPBUFFER;
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
	Variable *					ParseExp();
	void						ParseExpList();
	void						ParseStat();
	void						ParseElseif();
	void						ParseRet();

	Lua &Start();
	Lua &Assign(std::string var, const Variable &value);
	Lua &Call(std::string function);
	Lua &End();
};
