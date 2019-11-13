#include "lua.hpp"

#include <iostream>

constexpr char BY_FILE_NAME = 0b00000001;
char *file_name = nullptr;
char flags = 0;

void ParseParams(int argc, char **argv)
{
	for(int i = 1; i < argc; i++)
	{
		flags = flags | BY_FILE_NAME;
		file_name = argv[i];
	}
}

int  main(int argc, char **argv)
{
	ParseParams(argc, argv);

	Lua lua;
	lua.Init();

	if(flags & BY_FILE_NAME)
	{
		std::cout << "Lendo de arquivo: " << file_name << std::endl;
		lua.OpenFile(file_name);
	}

	return 0;
}
