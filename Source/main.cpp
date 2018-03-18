#include "Engine.h"

int main(int argc, char** argv)
{
	Engine engine(argc, argv);
	assert(engine.Init());
	assert(engine.LoadContent());
	assert(engine.Execute());
	return 0;
}