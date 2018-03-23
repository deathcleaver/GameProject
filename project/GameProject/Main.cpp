//#include "Game\Game.hpp"

#include <string>

#include "ExceptionHandling.hpp"

#include <conio.h>

#include "Engine/Engine.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Interfaces/IDataParser.hpp"
#include "Engine/Graphics/Gui.hpp"
#include "Engine/Graphics/Gui/Panel.hpp"
#include "Engine/Graphics/Gui/Label.hpp"
#include "Engine/Graphics/Gui/ProgressBar.hpp"
#include "Engine/Graphics/Gui/ScrollBar.hpp"

#include "Engine\Core\System.hpp"

#include "Game/Game.hpp"

#include <PhysicsEngine/PhysicsEngine.hpp>
#include <PhysicsEngine/Shapes/PlaneShape.hpp>
#include <PhysicsEngine/Shapes/SphereShape.hpp>

int main(int argc, char* argv[]) {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#if _DEBUG
	Engine::Core::Library renderDoc;
	printf("Press 1 to attach RenderDoc, any other key to ignore!\n");
	int c = _getch();
	if (c == 49) {
		if (renderDoc.loadLibrary("C:/Program Files/RenderDoc/x86/RenderDoc.dll")) {
			printf("RenderDoc successfully loaded!\nGPU debugging avalible!\n");
		} else {
			printf("Failed to load RenderDoc!\nGPU debugging not avaible!\n");
		}
	}
#endif

	initExceptionHandlers();

	float dt = 0;
	uint64_t clocks = 0U;
	Engine::System::HighResClock clock;

	CEngine* e = new CEngine();

	e->setAssetDataFolder("E:/GameProjectAssets/");

	Engine::Input::Input* in = Engine::Input::Input::GetInput();

	Game* game = new Game(e);

	printf("starting loop\n");

	PhysicsEngine physEng;

	StaticObject* groundPlane = physEng.createStaticObject();

	PlaneShape* ps = new PlaneShape();
	groundPlane->shape = ps;
	ps->normal = glm::vec3(0, 1, 0);
	ps->distance = -50.0F;

	RigidBody* collisionShape = physEng.createRigidBody();

	SphereShape* ss = new SphereShape();
	collisionShape->shape = ss;
	ss->center = glm::vec3(0, 1, 0);
	ss->radius= 1.0F;
	
	while (e->isRunning()) {

		physEng.update(dt);

		game->update(dt, clocks);
		e->clearBackBuffer();

		game->render();

		e->presentFrame();

		clocks = clock.tick();
		dt = clock.seconds();
	}

	printf("finished execution\n");
	delete game;

	delete e;


	delete ps;
	delete ss;

	physEng.freeStaticObject(groundPlane);
	physEng.freeRigidBody(collisionShape);

#if _DEBUG
	renderDoc.unloadLibrary();
#endif

	return 0;
}