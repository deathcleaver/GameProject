#ifndef IRENDERENGINE_HPP
#define IRENDERENGINE_HPP

#ifdef RENDERENGINE_EXPORTS
#define RENDERENGINE_API _declspec(dllexport)
#else
#define RENDERENGINE_API _declspec(dllimport)
#endif

#include "IRenderObject.hpp"
#include "IAnimatedMesh.hpp"
#include "ICamera.hpp"
#include "IShaderObject.hpp"
#include "IFrameBuffer.hpp"
#include "ITexture.hpp"

#include "IFont.hpp"
#include "IText.hpp"

#include "IWindow.hpp"

enum class RenderEngineType
{
	eRenderNone = -1,
	eRenderOpenGL = 0,
};

enum class SType
{
	sOpenGLWindowCreateInfo = 0,
	sRenderEngineCreateInfo = 1,
};

struct OpenGLWindowCreateInfo
{
	SType stype;
	int majorVersion;
	int minorVersion;

};

typedef void* (*PFN_EngineAllocator(size_t));
typedef void (*PFN_EngineDeallocator(void*));

struct AllocatorInfo
{
	PFN_EngineAllocator pfnEngineAlloc;
	PFN_EngineDeallocator pfnEngineDealloc;
};

struct RenderEngineCreateInfo
{
	SType stype;
	bool createRenderWindow;
	RenderEngineType renderEngineType;
	//AllocatorInfo globalAllocators;
	void* pNext;
};

class IRenderEngine
{

public:

	virtual void init(RenderEngineCreateInfo &createInfo) = 0;
	virtual void release() = 0;
	virtual void renderDebugFrame() = 0;

	virtual void setDepthTest(bool enable) = 0;
	
	virtual void clearStencil() = 0;

	virtual void setStencilTest(bool enable) = 0;
	virtual void stencilMask(unsigned int mask) = 0;
	virtual void stencilClear(int mask) = 0;
	virtual void stencilOp(unsigned int fail, unsigned int zfail, unsigned int zpass) = 0;
	virtual void stencilFunc(unsigned int func, int ref, unsigned int mask) = 0;

	virtual bool getGraphicsReset() const = 0;

	virtual void updateViewPort(int width, int height) = 0;

	virtual ICamera* createCamera() = 0;
	//virtual void setActiveCamera(ICamera * camera) = 0;
	//virtual void* getViewMatrixPtr() = 0;

	virtual IMesh* createMesh() = 0;
	virtual IAnimatedMesh* createAnimatedMesh() = 0;
	//virtual IRenderObject* createRenderObject() = 0;
	virtual IShaderObject* createShaderObject() = 0;
	virtual ITexture* createTexture() = 0;

	virtual IFrameBuffer* createFrameBuffer() = 0;

	virtual IText* createText() = 0;

	virtual IFont* createFont() = 0;

	virtual IWindow* getMainWindow() = 0;

	virtual bool isRenderObjectIsInFrustum(IRenderObject* renderObject) = 0;

	virtual size_t getMemoryUsage() const = 0;
	virtual size_t getMaxMemory() const = 0;

};


extern "C" RENDERENGINE_API IRenderEngine* CreateRenderEngine();

typedef IRenderEngine* (*CreateRenderEngineProc)();

#endif