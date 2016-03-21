#include "Engine.h"
#include <memory>

int main(int argc, char** argv)
{
	int ret = 0;
	{
		std::unique_ptr<Engine> engine(new Engine(argc, argv));
		if (engine->Init()) return 1;
		//if (engine->LoadContent()) return 2;

		ret = engine->Execute();
	}
	return ret;
}