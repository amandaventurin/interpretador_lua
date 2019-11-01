#include "lua.hpp"

#include <iostream>
#include <fstream>

void db()
{
	std::cout << "ASDF" << std::endl;
}

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
			out << "\"" << var.value.string << "\"" << std::flush;
			break;
		case Variable::lua_function:
		case Variable::lua_Cfunction:
			out << "Can't print functions" << std::flush;
			break;
	}
	return out;
}

std::ostream &operator<<(std::ostream &out, const VariablesList &list)
{
	VariablesList::ListElement *aux = list.head;
	while(aux)
	{
		out << *aux->value << std::flush;
		if(aux->next)
			out << ", " << std::flush;
		aux = aux->next;
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
		case lua_function:
			value.function = other.value.function;
			break;
	}
}

Variable::Variable(std::string text)
	: type(lua_string)
{
	value.string = text;
}

Variable::Variable(std::function<void()> *Cfunction)
	: type(lua_Cfunction)
{
	value.Cfunction = Cfunction;
}

void Variable::Equals(const Variable &other)
{
	type = other.type;
	switch(type)
	{
		case lua_number:
			value.number = other.value.number;
			break;
		case lua_string:
			value.string = other.value.string;
			break;
		case lua_function:
			value.function = other.value.function;
			break;
	}
}

void Variable::operator=(const Variable &other)
{
	Equals(other);
}

void Variable::operator()(void)
{
	if(type == lua_Cfunction)
	{
		(*value.Cfunction)();
	} else if(type == lua_function)
		(*value.function)();
}

Variable Variable::nil;

VariablesList::ListElement::ListElement(std::string name)
	: name(name), next(nullptr)
{
	value = new Variable();
}

VariablesList::ListElement::~ListElement()
{
	delete value;
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

Variable &VariablesList::operator[](int index)
{
	VariablesList::ListElement *aux = head;
	while(index > 0 && aux)
	{
		aux = aux->next;
		index--;
	}
	return *aux->value;
}

void VariablesList::PushBack(Variable *var)
{
	ListElement *aux = new ListElement("");
	aux->value = var;
	if(head)
	{
		last->next = aux;
		last = last->next;
	} else
	{
		head = aux;
		last = aux;
	}
}

void VariablesList::Reset()
{
	ListElement *aux (nullptr);
	while(head)
	{
		//FIXME: Possible memory leak
		head->value = nullptr;
		aux = head;
		head = head->next;
		delete aux;
	}
}

int VariablesList::Size()
{
	int sum = 0;
	ListElement *aux = head;
	while(aux)
	{
		sum++;
		aux = aux->next;
	}
	return sum;
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

void Scope::SetVariable(std::string variable_name, const Variable &value)
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
		ParseBlock();
	} else if(BUFFER == "while")
	{
		ParseBlock();
	} else
	{
		PutBack();
		std::vector<std::string> var_list = ParseVarList();
		NextToken();
		if(BUFFER == "=")
		{
			ParseExpList();
			for(int n = 0; n < var_list.size(); n++)
			{
				Assign(var_list[n], EXPBUFFER[n]);
			}
		} else if(BUFFER == "(")
		{
			ParseExpList();
			Call(var_list[0]);
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

Variable *Lua::ParseExp()
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
		return new Variable(BUFFER);
	}
	else
	{
		code->putback(temp);
		NextWord();
		return &current_scope->GetVariable(BUFFER);
	}
}

void Lua::ParseExpList()
{
	EXPBUFFER.Reset();
	while(true)
	{
		EXPBUFFER.PushBack(ParseExp());
		NextToken();
		if(BUFFER != ",")
		{
			PutBack();
			break;
		}
	}
}

void Lua::ParseRet()
{
	//FIXME: AQUI TA DANDO PROBLEMA, ACHO EU
	ParseExpList();
	current_scope->return_value = EXPBUFFER;
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

	std::function<void()> *print = new std::function<void()>([&]() {
		for(int n = 0; n < EXPBUFFER.Size(); n++)
			std::cout << EXPBUFFER[n] << std::endl;
	});
	Variable print_variable (print);
	global_scope->SetVariable("print", print_variable);

	return *this;
}

Lua &Lua::Assign(std::string var, const Variable &value)
{
	global_scope->SetVariable(var, value);
}

Lua &Lua::Call(std::string function)
{
	#include "Cfunctions.cpp"
	current_scope->GetVariable(function)();
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
