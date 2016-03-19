#include "Engine.h"

#include "stdio.h"

int main(int argc, char** argv)
{
	Engine* engine = new Engine(argc, argv);
	if (engine->Init()) return 1;
	return engine->Execute();
}