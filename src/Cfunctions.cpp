if(value.Cfunction == "print")
{
	for(int n = 0; n < exp_list.Size(); n++)
		std::cout << exp_list[n] << std::endl;
}
else if(value.Cfunction == "type"){
	switch (exp_list[0].type) {
		case lua_nil:
			return_variable->PushBack(new Variable("nil"));
			break;
		case lua_table:
			return_variable->PushBack(new Variable("table"));
			break;
		case lua_number:
			return_variable->PushBack(new Variable("number"));
			break;
		case lua_string:
			return_variable->PushBack(new Variable("string"));
			break;
		case lua_Cfunction:
			return_variable->PushBack(new Variable("cfunction"));
			break;
		case lua_function:
			return_variable->PushBack(new Variable("function"));
			break;
	}
}
