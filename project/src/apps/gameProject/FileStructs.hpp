#ifndef FILESTRUCTS_HPP
#define FILESTRUCTS_HPP

#include <cstdint>

class IGameDataObject {
public:

	enum Type {
		GENERIC,
		SCENE,
		RENDERLAYER,
		STATICOBJECT
	};

	virtual ~IGameDataObject() {};
	virtual void* getData() const = 0;
	virtual uint32_t getDataSize() const = 0;

	virtual Type getType() const = 0;
};

struct Version {
	uint32_t version;
};

struct Dependency {
	uint8_t dependencyLength;
	char* dependencyName;
};

struct DependencyInfoSave {
	uint8_t nrDependencies;
};

struct DependencyInfo : DependencyInfoSave {
	Dependency* dependencies;
};

struct EntrySave {
	uint32_t formID;
	uint32_t entrySize;
	char tag[4];
};

struct Entry : EntrySave {
	IGameDataObject* data;		//dataSize = entrySize - sizeof(Entry);
	bool deleteFlag;
};

struct EntryOffset {
	uint32_t formID;
	uint32_t offset;
};

struct GameFileHeader_V1 {
	Version fileVersion;
	uint16_t headerLength;
	uint8_t flags;
	uint8_t padding;
	DependencyInfoSave depInfo;
	uint32_t nrEntries;
};

struct GameFileHeader_V2 {
	Version fileVersion;
	uint16_t headerLength;
	uint8_t flags;
	uint8_t padding;
	DependencyInfoSave depInfo;
	uint32_t nrEntries;
	uint32_t startupSceneRef;
};

/*
* Storable structs
*/

// struct for render layer data
struct RenderLayerSaveData {
	//uint32_t nameLength;
	const char* name;
	char resolutionType;
	uint32_t width;
	uint32_t height;
	bool depthBuffer;
	bool stencilBuffer;
	char nrColorBuffers;
	uint32_t shaderProgramRef;
};

struct SceneSaveData {
	const char* name;
	float skyColor[4];
	bool hasFog;
	float fog[8];
	bool hasWater;
	uint32_t numGroups;
};

struct StaticObjectSaveData {
	const char* name;
	const char* meshFile;
	bool useCollision;
};

class IRenderLayerDataObject : public IGameDataObject {
public:
	virtual ~IRenderLayerDataObject() {};

	virtual RenderLayerSaveData* getRenderLayerData() = 0;
	virtual void setRenderLayerData(RenderLayerSaveData* data) = 0;

	// IGameDataObject
	virtual void* getData() const = 0;
	virtual uint32_t getDataSize() const = 0;
};

class ISceneDataObject : public IGameDataObject {
public:
	virtual ~ISceneDataObject() {};

	virtual SceneSaveData* getSceneData() = 0;
	virtual void setSceneData(SceneSaveData* data) = 0;

	// IGameDataObject
	virtual void* getData() const = 0;
	virtual uint32_t getDataSize() const = 0;
};

class IStaticObjectDataObject : public IGameDataObject {
public:
	virtual ~IStaticObjectDataObject() {};

	virtual StaticObjectSaveData* getStaticObjectData() = 0;
	virtual void setStaticObjectData(StaticObjectSaveData* data) = 0;

	// IGameDataObject
	virtual void* getData() const = 0;
	virtual uint32_t getDataSize() const = 0;
};

class IDataObjectConverter {
public:
	virtual ~IDataObjectConverter() {};

	virtual IRenderLayerDataObject* asRenderLayer(IGameDataObject*& dataObject) = 0;
	virtual ISceneDataObject* asSceneData(IGameDataObject*& dataObject) = 0;
	virtual IStaticObjectDataObject* asStaticObjectData(IGameDataObject*& dataObject) = 0;
};

#endif