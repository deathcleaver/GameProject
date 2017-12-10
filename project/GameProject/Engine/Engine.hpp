#ifndef ENGINE_HPP
#define ENGINE_HPP 
/// Internal Includes
#include "Core/LibraryLoader.hpp"
#include "Core/Console.hpp"
#include "Interfaces/IDataParser.hpp"
#include "Interfaces/IAssetManager.hpp"
#include "Physics/PhysicsEngine.hpp"
#include "Utils/MemoryBuffer.hpp"

#include "AssetManager.hpp"

/// External Includes
#include <RenderEngine/IRenderEngine.hpp>

/// Std Includes
#include <map>
#include <vector>

using namespace Engine;

enum class DataParsersTypes {
	MESH_PARSER,
	TEXURE_PARSER,
	// todo fill with more types
};

class CEngine {

public:

	CEngine();
	virtual ~CEngine();

	void registerDataParser(Interfaces::IDataParser* parser, DataParsersTypes parserType);

	//void testCallDataParsers() {
	//	std::map<DataParsersTypes, std::vector<Interfaces::IDataParser*>>::iterator it = dataParsers.begin();
	//	std::map<DataParsersTypes, std::vector<Interfaces::IDataParser*>>::iterator eit = dataParsers.end();

	//	for (it; it != eit; it++) {
	//		size_t size = it->second.size();

	//		for (size_t i = 0; i < size; i++) {
	//			Core::MemoryBuffer memBuff;
	//			it->second[i]->load(memBuff, nullptr);
	//		}
	//	}
	//}

	bool setAssetDataFolder(const char* folderPath);

	const bool isRunning() const;

	void update(const float dt);

	void clearBackBuffer();
	void clearDebug();
	void presentFrame();

	void writeDepth(float depthValue, glm::mat4 worldMat, glm::mat4 mdl);

	void renderFullQuad();

	Interfaces::IAssetManager* getAssetManager() const;

private:

	Core::Library renderEngineLib;
	Core::Console* console;
	IRenderEngine* renderEngine;
	IWindow* gameWindow;
	AssetManager* assetManager;

	std::map<DataParsersTypes, std::vector<Interfaces::IDataParser*>> dataParsers;

	Physics::PhysicsEngine physEngine;

	int windowWidth;
	int windowHeight;

	bool pauseFlag;
	bool running;

	IShaderObject* depthWriteShader;
	int depthValueLoc;
	int depthVpMatLoc;
	int depthMdlMatLoc;

	IMesh* fullQuad;

};

#endif