#include "lua.hpp"

#include <iostream>
#include <sstream>
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
		case Variable::lua_function:
			out << "Function: unknown" << std::flush;
			break;
		case Variable::lua_Cfunction:
			out << "C++ Function: " << var.value.Cfunction << std::flush;
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
	: type(lua_string), value(text)
{}

Variable::Variable(double number)
	: type(lua_number), value(number)
{}

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
		case lua_Cfunction:
			value.Cfunction = other.value.Cfunction;
			break;
	}
}

Variable *Variable::Cat(const Variable &other)
{
	if((type != lua_string && type != lua_number) || (other.type != lua_string && other.type != lua_number))
		return nullptr;

	std::string this_string, to_add;

	if(other.type == lua_number)
	{
		std::ostringstream sstream;
		sstream << other.value.number;
		to_add = sstream.str();
	} else
		to_add = other.value.string;

	if(type == lua_number)
	{
		std::ostringstream sstream;
		sstream << value.number;
		this_string = sstream.str();
	} else
		this_string = value.string;

	return new Variable(this_string + to_add);
}

Variable *Variable::Add(const Variable &other)
{
	if(type == lua_number && other.type == lua_number)
		return new Variable(value.number + other.value.number);
}

Variable *Variable::Sub(const Variable &other)
{
	if(type == lua_number && other.type == lua_number)
		return new Variable(value.number - other.value.number);
}

Variable *Variable::Mul(const Variable &other)
{
	if(type == lua_number && other.type == lua_number)
		return new Variable(value.number * other.value.number);
}

Variable *Variable::Div(const Variable &other)
{
	if(type == lua_number && other.type == lua_number)
		return new Variable(value.number / other.value.number);
}

Variable *Variable::Les(const Variable &var)
{
	if(type == lua_number)
		if(var.type == lua_number)
		{
			return ((value.number < var.value.number) ? new Variable(1) : new Variable());
		}
}

Variable *Variable::Gre(const Variable &var)
{
	if(type == lua_number)
		if(var.type == lua_number)
		{
			return ((value.number > var.value.number) ? new Variable(1) : new Variable());
		}
}

Variable *Variable::Leq(const Variable &var)
{
	if(type == lua_number)
		if(var.type == lua_number)
		{
			return ((value.number <= var.value.number) ? new Variable(1) : new Variable());
		}
}

Variable *Variable::Geq(const Variable &var)
{
	if(type == lua_number)
		if(var.type == lua_number)
		{
			return ((value.number >= var.value.number) ? new Variable(1) : new Variable());
		}
}

Variable *Variable::Dif(const Variable &var)
{
	if(type == lua_number)
		if(var.type == lua_number)
		{
			return ((value.number != var.value.number) ? new Variable(1) : new Variable());
		}
}

Variable *Variable::Equ(const Variable &var)
{
	if(type == lua_number)
		if(var.type == lua_number)
		{
			return ((value.number == var.value.number) ? new Variable(1) : new Variable());
		}
}

void Variable::operator=(const Variable &other)
{
	Equals(other);
}

void *Variable::operator()(void *expressions)
{
	VariablesList *return_variable = new VariablesList();
	VariablesList &exp_list = *(VariablesList *)expressions;
	if(type == lua_Cfunction)
	{
		#include "Cfunctions.cpp"
	} else if(type == lua_function)
	{
		return_variable->PushBack((*value.function)(expressions));
	}

	return return_variable;
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
	if(variables.Find(variable_name) || parent == this)
		return variables[variable_name];
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
	: global_scope(nullptr), current_scope(nullptr), code(nullptr), end(false), first_scope(true)
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

#include "states.cpp"

void Lua::RegisterCFunction(std::string text)
{
	Variable *var = new Variable();
	var->type = Variable::lua_Cfunction;
	var->value.Cfunction = text;
	global_scope->SetVariable(text, *var);
}

Lua &Lua::Init()
{
	global_scope = new Scope();
	current_scope = global_scope;
	global_scope->parent = global_scope;

	RegisterCFunction("print");
	RegisterCFunction("type");

	return *this;
}

Lua &Lua::BeginScope()
{
	if(first_scope)
		first_scope = false;
	else
	{
		Scope *aux = new Scope();
		aux->parent = current_scope;
		current_scope = aux;
	}
}

Lua &Lua::EndScope()
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
