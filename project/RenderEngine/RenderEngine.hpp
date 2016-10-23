#ifndef RENDERENGINE_HPP
#define RENDERENGINE_HPP

#include "Window\Window.hpp"
#include "Window\WindowSystem.hpp"

#undef RGB

#include "IRenderEngine.hpp"
#include "DataTypes\Colors.hpp"
#include "ShaderState.hpp"
#include "Assets\RenderObject.hpp"
#include "Camera.hpp"

#include <Allocators\FrameAllocator.hpp>
#include <Allocators\PoolAllocator.hpp>

#include <freetype\ft2build.h>
#include FT_FREETYPE_H


#include <vld.h>

struct GLinfo
{
	const char* vendor;
	const char* renderer;
	const char* version;
	const char* shadingLanguageVersion;
};

class RenderEngine : public IRenderEngine
{
public:

	RenderEngine() {};

	virtual void init(RenderEngineCreateInfo &createInfo);
	virtual void release();
	virtual void renderDebugFrame();

	virtual void setDepthTest(bool enable);

	virtual void clearStencil();

	virtual void setStencilTest(bool enable);
	virtual void stencilMask(unsigned int mask);
	virtual void stencilClear(int mask);
	virtual void stencilOp(unsigned int fail, unsigned int zfail, unsigned int zpass);
	virtual void stencilFunc(unsigned int func, int ref, unsigned int mask);

	virtual bool getGraphicsReset() const;

	virtual void updateViewPort(int width, int height);

	virtual ICamera* createCamera();
	virtual void setActiveCamera(ICamera * camera);
	//virtual void* getViewMatrixPtr();

	virtual IMesh* createMesh();
	virtual IAnimatedMesh* createAnimatedMesh();
	//virtual IRenderObject* createRenderObject();
	virtual IShaderObject* createShaderObject();
	virtual ITexture* createTexture();

	virtual IFrameBuffer* createFrameBuffer();

	virtual IText* createText();

	virtual IFont* createFont();

	virtual bool isRenderObjectIsInFrustum(IRenderObject* renderObject);
	
	virtual size_t getMemoryUsage() const;
	virtual size_t getMaxMemory() const;

private:

	int counter = 0;
	
	GLinfo info;

	RGB clearColor;
	ShaderState shaderState;
	Camera cam;

	PoolAllocator<RenderObject> objectPool;
	//FrameAllocator frameAlloc;

	FT_Library fontLibrary;


	GLWindow glWindow;

	RenderEngineCreateInfo reci;

};

#endif