// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <core/game/game.h>
#include "../my_scenes/simple_scene.h"

int main(void)
{
	Game game(1024, 768, WindowRendererType::OpenGL);
	SimpleScene* simple_scene = new SimpleScene("A Simple Scene");
	game.PlayMainScene(simple_scene);
	delete simple_scene;
}
