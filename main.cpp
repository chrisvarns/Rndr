#include "Globals.h"
#include "Engine.h"

#include "stdio.h"

int main(int argc, char** argv)
{
	Engine* engine = new Engine(argc, argv);
	if (engine->Init()) return gs_Fail;
	return engine->Execute();
}