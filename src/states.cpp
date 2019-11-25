void db(){std::cout << "ASDF" << std::endl;}

void Lua::ParseStat()
{
	if(BUFFER == "func")
	{
		ParseBlock();
	} else if(BUFFER == "while")
	{
		ParseBlock();
	} else if(BUFFER == "if")
	{
		Variable *res = ParseExp();
		NextWord();
		if(BUFFER == "then"){
			if(res->type != Variable::lua_nil)
			{
				ParseBlock();
			} else
			{
				bool can_ifs = true;
				do{
					NextWord();
					if(can_ifs && BUFFER == "elseif")
						can_ifs = !ParseElseif();
				}while(BUFFER != "else" && BUFFER != "end");

				if(BUFFER == "else" && can_ifs)
					ParseBlock();

				while(BUFFER != "end")
					NextWord();
			}
		}
	} else
	{
		PutBack();
		ParseVarList();
		NextToken();
		if(BUFFER == "=")
		{
			VariablesList *exp_list = ParseExpList();
			for(int n = 0; n < VARBUFFER.Size(); n++)
			{
				VARBUFFER[n] = (*exp_list)[n];
			}
		} else if(BUFFER == "(")
		{
			ParseFunctionCall(VARBUFFER[0]);
		}
	}
}

VariablesList *Lua::ParseFunctionCall(Variable &var)
{
	return (VariablesList *)var(ParseExpList());
}

bool Lua::ParseElseif(){
	Variable *res = ParseExp();
	NextWord();
	if(BUFFER == "then" && res->type != Variable::lua_nil){
		ParseBlock();
		return true;
	}
	return false;
}

Variable *Lua::ParseVar()
{
	NextWord();
	return &current_scope->GetVariable(BUFFER);
}

void Lua::ParseVarList()
{
	VARBUFFER.Reset();
	while(true)
	{
		VARBUFFER.PushBack(ParseVar());
		NextToken();
		if(BUFFER != ",")
		{
			PutBack();
			break;
		}
	}
}

Variable *Lua::ParseExp()
{
	Variable *return_variable { nullptr };
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
			else if(temp == '\\')
			{
				(*code) >> std::noskipws >> temp;
				if(temp == 'n')
					BUFFER += '\n';
				else if(temp == 't')
					BUFFER += '\t';
				else if(temp == 'r')
					BUFFER += '\r';
			}
			else
				BUFFER += temp;
		}
		return_variable = new Variable(BUFFER);
	} else if(temp >= '0' && temp <= '9')
	{
		code->putback(temp);
		BUFFER = "";
		while(true)
		{
			(*code) >> std::noskipws >> temp;
			if(isSpace(temp) || (temp != '.' && temp != '-' && isToken(temp)))
				break;
			else
				BUFFER += temp;
		}
		code->putback(temp);
		return_variable = new Variable(stod(BUFFER));
	} else if(temp == '(')
	{
		return_variable = ParseExp();
	} else
	{
		code->putback(temp);
		NextWord();
		if(BUFFER == "nil") return_variable = &Variable::nil;
		else return_variable = &current_scope->GetVariable(BUFFER);
	}

	(*code) >> std::skipws >> temp;
	if(temp == ')')
		return return_variable;
	code->putback(temp);

	if(return_variable->type == Variable::lua_string)
	{
		NextToken();
		if(BUFFER == "..")
			return_variable = return_variable->Cat(*ParseExp());
		else
			PutBack();
	} else if(return_variable->type == Variable::lua_number)
	{
		NextToken();
		if(BUFFER == "+")
		{
			return_variable = return_variable->Add(*ParseExp());
		} else if(BUFFER == "-")
		{
			return_variable = return_variable->Sub(*ParseExp());
		} else if(BUFFER == "*")
		{
			return_variable = return_variable->Mul(*ParseExp());
		} else if(BUFFER == "/")
		{
			return_variable = return_variable->Div(*ParseExp());
		} else if(BUFFER == "..")
		{
			return_variable = return_variable->Cat(*ParseExp());
		} else if(BUFFER == "<")
		{
			return_variable = return_variable->Les(*ParseExp());
		} else if(BUFFER == ">")
		{
			return_variable = return_variable->Gre(*ParseExp());
		} else if(BUFFER == "<=")
		{
			return_variable = return_variable->Leq(*ParseExp());
		} else if(BUFFER == ">=")
		{
			return_variable = return_variable->Geq(*ParseExp());
		} else if(BUFFER == "~=")
		{
			return_variable = return_variable->Dif(*ParseExp());
		} else if(BUFFER == "=")
		{
			return_variable = return_variable->Equ(*ParseExp());
		} else
		{
			PutBack();
		}
	} else if(return_variable->type == Variable::lua_Cfunction || return_variable->type == Variable::lua_function)
	{
		NextToken();
		if(BUFFER == "(")
			return_variable = &(*ParseFunctionCall(*return_variable))[0];
		else
			PutBack();
	}

	return return_variable;
}

VariablesList *Lua::ParseExpList()
{
	VariablesList *return_variable = new VariablesList();
	while(true)
	{
		return_variable->PushBack(ParseExp());
		NextToken();
		if(BUFFER != ",")
		{
			PutBack();
			break;
		}
	}
	return return_variable;
}

void Lua::ParseRet()
{
	current_scope->return_value = *ParseExpList();
}

void Lua::ParseBlock()
{
	BeginScope();

	while(!end)
	{
		NextWord();
		if(BUFFER == "return" || BUFFER == "end" || BUFFER == "elseif" || BUFFER == "else")
			break;
		else
			ParseStat();
	}

	if(BUFFER == "return") ParseRet();

	EndScope();
}
