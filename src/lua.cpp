#include "lua.hpp"

#include <iostream>
#include <fstream>

int min(int n1, int n2)
{
	return (n1 < n2) ? n1 : n2;
}

std::ostream &operator<<(std::ostream &out, const Variable &var)
{
	switch(var.type)
	{
		case Variable::lua_nil:
			out << "nil" << std::flush;
			break;
		case Variable::lua_number:
			out << var.value.number << std::flush;
			break;
		case Variable::lua_string:
			out << var.value.string << std::flush;
			break;
	}
	return out;
}

std::ostream &operator<<(std::ostream &out, const std::vector<std::string> &list)
{
	for(int n = 0; n < list.size(); n++)
	{
		out << list[n] << std::flush;
		if(n != list.size() - 1)
			out << ", " << std::flush;
	}
	return out;
}

bool isSpace(char ch)
{
	return (
		ch == ' ' ||
		ch == '\t' ||
		ch == '\n');
}

bool isToken(char ch)
{
	return (
		ch == '~' ||
		ch == '<' ||
		ch == '>'||
		ch == '='||
		ch == '+'||
		ch == '-'||
		ch == '/'||
		ch == '*'||
		ch == '%'||
		ch == '{'||
		ch == '}'||
		ch == '('||
		ch == ')'||
		ch == '['||
		ch == ']'||
		ch == ';'||
		ch == ','||
		ch == '@'||
		ch == '.');
}

Variable::Variable()
	: type(lua_nil)
{}

Variable::Variable(const Variable &other)
	: type(other.type)
{
	switch(type)
	{
		case lua_number:
			value.number = other.value.number;
			break;
		case lua_string:
			value.string = other.value.string;
			break;
	}
}

Variable::Variable(std::string text)
	: type(lua_string), value(text)
{}

void Variable::Equals(std::string var)
{
	type = lua_string;
	value.string = var;
}

Variable Variable::nil;

VariablesList::ListElement::ListElement(std::string name)
	: name(name), next(nullptr)
{
	value = new Variable();
}

VariablesList::VariablesList()
	: head(nullptr)
{}

VariablesList::~VariablesList()
{
	ListElement *aux;
	while(head)
	{
		aux = head;
		head = head->next;
		delete aux;
	}
}

Variable *VariablesList::Find(std::string name)
{
	VariablesList::ListElement *aux = head;
	while(aux)
	{
		if(aux->name == name)
			return aux->value;
		aux = aux->next;
	}
	return nullptr;
}

Variable &VariablesList::operator[](std::string name)
{
	if(Find(name))
	{
		return *Find(name);
	}
	else
	{
		if(head)
		{
			last->next = new ListElement(name);
			last = last->next;
		} else
		{
			head = new ListElement(name);
			last = head;
		}
		return *last->value;
	}
}

Scope::Scope()
	: parent(nullptr)
{}

Variable &Scope::GetVariable(std::string variable_name)
{
	if(variables.Find(variable_name))
		return variables[variable_name];
	else if(parent == this)
		return Variable::nil;
	else
		return parent->GetVariable(variable_name);
}

void Scope::SetVariable(std::string variable_name, std::string value)
{
	variables[variable_name].Equals(value);
}

void Lua::NextWord()
{
	char temp;
	BUFFER = "";
	while(true)
	{
		(*code) >> std::noskipws >> temp;
		if(code->eof())
		{
			if(BUFFER == "")
				BUFFER = "end";
			end = true;
			break;
		}

		if(!isSpace(temp) && !isToken(temp))
			BUFFER += temp;
		else if(BUFFER != "")
		{
			code->putback(temp);
			break;
		}
	}
}

void Lua::NextToken()
{
	char temp;
	BUFFER = "";
	while(true)
	{
		(*code) >> std::noskipws >> temp;
		if(code->eof())
		{
			end = true;
			break;
		}

		if(isToken(temp))
			BUFFER += temp;
		else if(BUFFER != "" || !isSpace(temp))
		{
			code->putback(temp);
			break;
		}
	}
}

void Lua::PutBack()
{
	for(int n = BUFFER.size() - 1; n >= 0; n--)
	{
		code->putback(BUFFER[n]);
	}
	BUFFER = "";
}

Lua::Lua()
	: global_scope(nullptr), current_scope(nullptr), code(nullptr), end(false)
{}

Lua::~Lua()
{
	global_scope = nullptr;
	while(current_scope)
	{
		Scope *aux = current_scope;
		current_scope = current_scope->parent;
		delete aux;
	}
	current_scope = nullptr;
}

void Lua::OpenFile(std::string file_name)
{
	std::filebuf file;
	if(file.open (file_name, std::ios::in))
	{
		ParseStream(&file);
		file.close();
	} else
	{
		std::cout << "Arquivo não encontrado: " << file_name << std::endl;
	}
}

void Lua::ParseStream(std::streambuf *code_ptr)
{
	code = new std::iostream(code_ptr);
	ParseBlock();
	delete code;
}

void Lua::ParseStat()
{
	if(BUFFER == "func")
	{
		std::cout << "Defining a new function" << std::endl;
		ParseBlock();
	} else if(BUFFER == "while")
	{
		std::cout << "Entering a while loop" << std::endl;
		ParseBlock();
	} else
	{
		PutBack();
		std::vector<std::string> var_list = ParseVarList();
		NextToken();
		if(BUFFER == "=")
		{
			std::vector<std::string> values = ParseExpList();
			for(int n = 0; n < min(var_list.size(), values.size()); n++)
			{
				Assign(var_list[n], values[n]);
			}
		} else if(BUFFER == "(")
		{
			std::vector<std::string> parameters = ParseExpList();
			Call(var_list[0], parameters);
		}
	}
}

std::string Lua::ParseVar()
{
	NextWord();
	return BUFFER;
}

std::vector<std::string> Lua::ParseVarList()
{
	std::vector<std::string> return_value;
	while(true)
	{
		return_value.push_back(ParseVar());
		NextToken();
		if(BUFFER != ",")
		{
			PutBack();
			break;
		}
	}
	return return_value;
}

std::string Lua::ParseExp()
{
	char temp;
	(*code) >> std::skipws >> temp;
	if(temp == '\"' || temp == '\'')
	{
		char exit_condition = temp;
		BUFFER = "";
		while(true)
		{
			(*code) >> std::noskipws >> temp;
			if(temp == exit_condition)
				break;
			else
				BUFFER += temp;
		}
	}
	else
	{
		code->putback(temp);
		NextWord();
	}
	return BUFFER;
}

std::vector<std::string> Lua::ParseExpList()
{
	std::vector<std::string> return_value;
	while(true)
	{
		return_value.push_back(ParseExp());
		NextToken();
		if(BUFFER != ",")
		{
			PutBack();
			break;
		}
	}
	return return_value;
}

void Lua::ParseRet()
{
	current_scope->return_value = ParseExpList();
}

void Lua::ParseBlock()
{
	while(!end)
	{
		NextWord();
		if(BUFFER == "return" || BUFFER == "end")
			break;
		else
			ParseStat();
	}

	if(BUFFER == "return") ParseRet();

	End();
}

Lua &Lua::Start()
{
	global_scope = new Scope();
	current_scope = global_scope;
	global_scope->parent = global_scope;
	return *this;
}

Lua &Lua::Assign(std::string var, std::string value)
{
	global_scope->SetVariable(var, value);
}

Lua &Lua::Call(std::string function, const std::vector<std::string> &parameters)
{
	if(function == "print")
		for(const std::string &var : parameters)
			std::cout << current_scope->GetVariable(var) << std::endl;
}

Lua &Lua::End()
{
	if(current_scope == global_scope)
	{
		delete global_scope;
		global_scope = nullptr;
		current_scope = nullptr;
	} else
	{
		Scope *aux = current_scope;
		current_scope = current_scope->parent;
		delete current_scope;
	}
}
