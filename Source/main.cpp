#include "Engine.h"

int main(int argc, char** argv)
{
	Engine engine(argc, argv);
	if (!engine.Init()) return 1;
	if (!engine.LoadContent()) return 2;
	if (!engine.Execute()) return 3;
	return 0;
}