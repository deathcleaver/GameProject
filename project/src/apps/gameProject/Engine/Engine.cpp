/// Internal Includes

#include "Engine.hpp"
#include "Graphics/Graphics.hpp"
#include "Input/Input.hpp"
#include "Graphics/GuiTheme.hpp"
#include "Graphics/GraphicsResources/arrow_left.png.h"
#include "Graphics/GraphicsResources/arrow_right.png.h"

/// External Includes
#include <Core/LibraryLoader.hpp>
#include <Core/System.hpp>

#include <AssetLib/AssetLib.hpp>

/// Std Includes

const char* vertexShader = ""
"#version 410\n"
"\n"
"layout(location = 0) in vec3 vertexPos;\n"
"uniform mat4 vp;\n"
"uniform mat4 mdl;\n"
"\n"
"void main() {\n"
"	vec3 p = vertexPos;\n"
"	vec3 pos = (vec4(p, 1.0) * mdl).xyz;\n"
"	gl_Position = vp * vec4(pos, 1.0);\n"
"}\n"
"\n";

const char* fragmentShader = ""
"#version 410\n"
"\n"
"out float gl_FragDepth;\n"
"uniform float depthValue;\n"
"\n"
"void main() {\n"
"gl_FragDepth = depthValue;"
"}\n"
"\n";

CEngine::CEngine()
	: guiInfo()
	, gui(nullptr)
	, cursor(nullptr)
	, cursorTexture(nullptr)
	, threadManager(nullptr)
	, physEngine(nullptr)
	, groundPlane(nullptr)
	, engineSettings()
	, renderEngineLib()
	, console(nullptr)
	, renderEngine(nullptr)
	, gameWindow(nullptr)
	, assetManager(nullptr)
	, windowWidth(0)
	, windowHeight(0)
	, pauseFlag(false)
	, running(true)
	, depthWriteShader(nullptr)
	, depthValueLoc(0)
	, depthVpMatLoc(0)
	, depthMdlMatLoc(0)
	, fullscreenQuad(nullptr)
{

	console = new Engine::Core::Console();
	Engine::Core::gConsole = console;

	Engine::System::initSys();

	// this must be called before using renderEngine
	loadRenderEngine();
	loadSettings();
	
	createGameWindow();

	assetManager = new Engine::AssetHandling::AssetManager(renderEngine);
	setupGui(assetManager);

	console->initGraphics(gui);
	console->updateSize(windowWidth, windowHeight);

	//gameWindow->setCursorVisibility(false);

	setupInput(gameWindow, console);

	//gameWindow->lockCursor(true);

	createDepthShader();

	fullscreenQuad = new Engine::Graphics::FullscreenQuad();


	threadManager = new ThreadManager();

	setupPhysicsEngine();

	gTheme = nullptr;
	initTheme();
}

CEngine::~CEngine() {


	delete groundPlane->shape;
	physEngine->freeStaticObject(groundPlane);

	delete threadManager;

	delete physEngine;

	delete assetManager;

	depthWriteShader->release();

	delete fullscreenQuad;

	delete console;

	delete cursor;
	delete gui;
	cleanupTheme();

	gRenderEngine = nullptr;

	Engine::Input::Input::Release();

	renderEngine->release();
	renderEngineLib.unloadLibrary();

	Engine::System::deinitSys();

	console = nullptr;
	renderEngine = nullptr;
	gameWindow = nullptr;
}

void CEngine::setCursor(char* ) {

	//cursorTexture = assetManager->loadTexture(cursorImgPath);
	//cursor->setTexture(cursorTexture);
}

void CEngine::close() {
	gameWindow->showWindow(false);
}

Engine::Graphics::CGui* CEngine::getGui() const {
	return gui;
}

bool CEngine::setAssetDataFolder(const char* folderPath) {
	bool success = false;
	if (Engine::System::folderExists(folderPath)) {
		gAssetDataPath = folderPath;
		success = true;
	}
	return success;
}

const bool CEngine::isRunning() const {
	return running;
}

void CEngine::update(const float dt) {
	Engine::Input::Input::GetInput()->reset();

	gameWindow->pollMessages();

	physEngine->update(dt);

	handleWindowSizeChange();

	console->update(dt);
	gui->update(dt);

	running = gameWindow->isVisible();
}

void CEngine::lockCursor(bool lock) {
	gameWindow->lockCursor(lock);
}

void CEngine::clearBackBuffer() {
	renderEngine->bindDefaultFrameBuffer();
	renderEngine->clear();
}

void CEngine::clearDebug() {
	renderEngine->bindDefaultFrameBuffer();
	renderEngine->clearDebug();
}

void CEngine::presentFrame() {
	gui->render();
	gameWindow->swapBuffers();
}

void CEngine::writeDepth(float depthValue, glm::mat4 vpMat, glm::mat4 mdl) {
	depthWriteShader->useShader();
	depthWriteShader->bindData(depthValueLoc, UniformDataType::UNI_FLOAT, &depthValue);
	depthWriteShader->bindData(depthVpMatLoc, UniformDataType::UNI_MATRIX4X4, &vpMat);
	depthWriteShader->bindData(depthMdlMatLoc, UniformDataType::UNI_MATRIX4X4, &mdl);
}

void CEngine::renderFullQuad() {
	fullscreenQuad->render();
}

Engine::AssetHandling::IAssetManager* CEngine::getAssetManager() const {
	return assetManager;
}

PhysicsEngine* CEngine::getPhysEngine() {
	return physEngine;
}

ThreadManager* CEngine::getThreadManager() {
	return threadManager;
}

inline void CEngine::loadRenderEngine() {

	if (renderEngineLib.loadLibrary("RenderEngine.dll\0")) {
		Engine::Core::gConsole->print("Loaded RenderEngine.dll");
	} else {
		Engine::Core::gConsole->print("Failed to load Renderer");
		throw;
	}
}

inline void CEngine::loadSettings() {
	windowWidth = engineSettings.resolution().width;
	windowHeight = engineSettings.resolution().height;
}

inline void CEngine::createGameWindow() {

	CreateRenderEngineProc rProc = (CreateRenderEngineProc)renderEngineLib.getProcAddress("CreateRenderEngine");

	RenderEngineCreateInfo reci;
	reci.stype = SType::sRenderEngineCreateInfo;
	reci.createRenderWindow = true;
	reci.renderEngineType = RenderEngineType::eRenderOpenGL;
	reci.pNext = nullptr;

	renderEngine = rProc();
	renderEngine->init(reci);
	renderEngine->updateViewPort(windowWidth, windowHeight);

	gRenderEngine = renderEngine;

	gameWindow = renderEngine->getMainWindow();

	gameWindow->setWindowSize(windowWidth, windowHeight);
	gameWindow->setVsync(engineSettings.vSync());
	gameWindow->setVsync(false);
}

inline void CEngine::setupInput(IWindow* window, Engine::Core::Console* console_in) {

	Engine::Input::Input* input = Engine::Input::Input::GetInput();

	input->setupCallbacks(window);
	input->attachConsole(console_in);
}

inline void CEngine::setupGui(Engine::AssetHandling::AssetManager* assetManager_in) {

	guiInfo.pAssetManager = assetManager_in;

	gui = new Engine::Graphics::CGui(assetManager_in);
	gui->setVisible(true);

	cursor = new Engine::Graphics::Gui::Cursor(guiInfo);
	cursor->setSize(25, 25);
	cursor->setAnchorPoint(Engine::Graphics::GuiAnchor::TOP_LEFT);
	cursor->setVisible(false);

	gui->setCursor(cursor);
}

inline void CEngine::handleWindowSizeChange() {

	if (Engine::Input::Input::GetInput()->sizeChange) {
		int w = 0;
		int h = 0;
		Engine::Input::Input::GetInput()->getWindowSize(w, h);

		renderEngine->updateViewPort(w, h);
		gameWindow->setWindowSize(w, h);

		console->updateSize(w, h);

		windowHeight = h;
		windowWidth = w;
	}
}

inline void CEngine::createDepthShader() {

	depthWriteShader = gRenderEngine->createShaderObject();
	depthWriteShader->init();

	depthWriteShader->setShaderCode(ShaderStages::VERTEX_STAGE, vertexShader);
	depthWriteShader->setShaderCode(ShaderStages::FRAGMENT_STAGE, fragmentShader);

	if (!depthWriteShader->buildShader()) {
		assert(0 && "Failed to build ClearDepth shader!");
	}

	depthValueLoc = depthWriteShader->getShaderUniform("depthValue");
	depthVpMatLoc = depthWriteShader->getShaderUniform("vp");
	depthMdlMatLoc = depthWriteShader->getShaderUniform("mdl");
}

inline void CEngine::setupPhysicsEngine() {
	PhysEngineCreateInfo peci;

	peci.threaded = true;
	peci.taskMgr = threadManager;
	peci.maxTasks = Engine::System::getLogicalProcessorCount();

	physEngine = new PhysicsEngine(peci);

	groundPlane = physEngine->createStaticObject();

	PlaneShape* ps = new PlaneShape();
	groundPlane->shape = ps;
	ps->normal = glm::vec3(0, 1, 0);
	ps->distance = -5.0F;
}

inline void CEngine::initTheme() {

	gTheme = new Engine::Theme::GuiTheme();

	gTheme->button.textureNormal = new Engine::Graphics::Texture::Texture2D();
	gTheme->button.textureHovering = new Engine::Graphics::Texture::Texture2D();
	gTheme->button.texturePressing = new Engine::Graphics::Texture::Texture2D();
	gTheme->button.textureNormal->singleColor(0.6F, 0.6F, 0.6F, 1.0F);
	gTheme->button.textureHovering->singleColor(0.55F, 0.55F, 0.55F, 1.0F);
	gTheme->button.texturePressing->singleColor(0.046F, 0.51F, 1.0F, 1.0F);

	gTheme->comboBox.textureNormal = new Engine::Graphics::Texture::Texture2D();

	gTheme->list.textureBackground = new Engine::Graphics::Texture::Texture2D();
	gTheme->list.textureItem = new Engine::Graphics::Texture::Texture2D();
	gTheme->list.textureSelectedItem = new Engine::Graphics::Texture::Texture2D();
	gTheme->list.areaScroll.textureBackground = new Engine::Graphics::Texture::Texture2D();
	gTheme->list.areaScroll.textureBar = new Engine::Graphics::Texture::Texture2D();
	gTheme->list.textureBackground->singleColor(0.65F, 0.65F, 0.65F, 1.0F);
	gTheme->list.textureSelectedItem->singleColor(0.046F, 0.51F, 1.0F, 1.0F);
	gTheme->list.textureItem->singleColor(0.60F, 0.60F, 0.60F, 1.0F);

	gTheme->panel.textureNormal = new Engine::Graphics::Texture::Texture2D();
	gTheme->panel.textureNormal->singleColor(0.5F, 0.5F, 0.5F, 1.0F);

	gTheme->progressBar.textureBackground = new Engine::Graphics::Texture::Texture2D();
	gTheme->progressBar.textureBar = new Engine::Graphics::Texture::Texture2D();

	gTheme->scrollBar.textureBackground = new Engine::Graphics::Texture::Texture2D();
	gTheme->scrollBar.textureBar = new Engine::Graphics::Texture::Texture2D();

	gTheme->scrollBar.textureBackground->singleColor(0.7F, 0.7F, 0.7F, 1.0F);
	gTheme->scrollBar.textureBar->singleColor(0.3F, 0.3F, 0.3F, 1.0F);

	gTheme->textArea.textureBackground = new Engine::Graphics::Texture::Texture2D();
	gTheme->textArea.areaScroll.textureBackground = new Engine::Graphics::Texture::Texture2D();
	gTheme->textArea.areaScroll.textureBar = new Engine::Graphics::Texture::Texture2D();

	gTheme->textArea.textureBackground->singleColor(0.3F, 0.3F, 0.3F, 1.0F);
	gTheme->textArea.areaScroll.textureBackground->singleColor(1.0F, 1.0F, 1.0F, 1.0F);
	gTheme->textArea.areaScroll.textureBar->singleColor(1.0F, 1.0F, 1.0F, 1.0F);

	gTheme->statusBar.textureBackground = new Engine::Graphics::Texture::Texture2D();
	gTheme->statusBar.textureBackground->singleColor(0.0F, 0.0F, 0.0F, 0.7F);

	gTheme->tabView.background = new Engine::Graphics::Texture::Texture2D();
	gTheme->tabView.background->singleColor(0.0F, 0.0F, 0.0F, 0.7F);

	int w, h, c;
	void* data = AssetLib::loadTextureFromMemory((void*)arrow_right_x32, sizeof(arrow_right_x32), w, h, c);

	gTheme->tabView.icoNext = new Engine::Graphics::Texture::Texture2D();
	gTheme->tabView.icoNext->setData(w, h, c, data);
	AssetLib::freeImageData(data);

	data = AssetLib::loadTextureFromMemory((void*)arrow_left_x32, sizeof(arrow_left_x32), w, h, c);
	gTheme->tabView.icoPrev = new Engine::Graphics::Texture::Texture2D();
	gTheme->tabView.icoPrev->setData(w, h, c, data);
	AssetLib::freeImageData(data);
}

template<typename T>
inline void deleteAndNullify(T** ptr) {
	delete *ptr;
	*ptr = nullptr;
}

inline void CEngine::cleanupTheme() {

	if (gTheme != nullptr) {
		deleteAndNullify(&gTheme->button.textureNormal);
		deleteAndNullify(&gTheme->button.textureHovering);
		deleteAndNullify(&gTheme->button.texturePressing);

		deleteAndNullify(&gTheme->comboBox.textureNormal);

		deleteAndNullify(&gTheme->list.textureBackground);
		deleteAndNullify(&gTheme->list.textureItem);
		deleteAndNullify(&gTheme->list.textureSelectedItem);
		deleteAndNullify(&gTheme->list.areaScroll.textureBackground);
		deleteAndNullify(&gTheme->list.areaScroll.textureBar);

		deleteAndNullify(&gTheme->panel.textureNormal);

		deleteAndNullify(&gTheme->progressBar.textureBackground);
		deleteAndNullify(&gTheme->progressBar.textureBar);

		deleteAndNullify(&gTheme->scrollBar.textureBackground);
		deleteAndNullify(&gTheme->scrollBar.textureBar);

		deleteAndNullify(&gTheme->textArea.textureBackground);
		deleteAndNullify(&gTheme->textArea.areaScroll.textureBackground);
		deleteAndNullify(&gTheme->textArea.areaScroll.textureBar);
		
		deleteAndNullify(&gTheme->statusBar.textureBackground);

		deleteAndNullify(&gTheme->tabView.background);
		deleteAndNullify(&gTheme->tabView.icoNext);
		deleteAndNullify(&gTheme->tabView.icoPrev);

	}
	deleteAndNullify(&gTheme);
}