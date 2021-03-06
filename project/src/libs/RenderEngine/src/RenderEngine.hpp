#ifndef RENDERENGINE_HPP
#define RENDERENGINE_HPP

#include "Window/Window.hpp"
#include "Window/WindowSystem.hpp"

#undef RGB

#include "RenderEngine/IRenderEngine.hpp"
#include "DataTypes/Colors.hpp"

//#include <Allocators\FrameAllocator.hpp>
//#include <Allocators\PoolAllocator.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

//#include <vld.h>

struct GLinfo {
	const char* vendor;
	const char* renderer;
	const char* version;
	const char* shadingLanguageVersion;
};

struct VKinfo {
	const char* vendor;
	const char* renderer;
	const char* version;
};

struct DisplaySize {
	float width;
	float height;
};

class RenderEngine : public IRenderEngine {
public:

	RenderEngine() {};

	virtual void init(RenderEngineCreateInfo &createInfo);
	virtual void release();
	virtual void clearDebug();
	virtual void clear();
	virtual void bindDefaultFrameBuffer();

	virtual void enableCulling(bool enable, bool backFace = true);

	virtual void setDepthTest(bool enable);
	virtual void depthMask(bool mask);
	virtual void forceWriteDepth(bool forceWrite);

	virtual void colorMask(bool r, bool g, bool b, bool a);

	virtual const ScissorInfo pushScissorRegion(int x, int y, int width, int height);
	virtual void popScissorRegion(const ScissorInfo prevScissor);

	virtual void clearStencil();

	virtual void setStencilTest(bool enable);
	virtual void stencilMask(unsigned int mask);
	virtual void stencilClear(int mask);
	virtual void stencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass);
	virtual void stencilFunc(FuncConstants func, int ref, unsigned int mask);
	virtual void setClearColor(float r, float g, float b, float a);

	virtual void activeTexture(int slot);

	virtual void setBlending(bool enable);

	virtual bool getGraphicsReset() const;

	virtual void updateViewPort(int width, int height);

	virtual IMesh* createMesh();
	virtual IAnimatedMesh* createAnimatedMesh();
	virtual IShaderObject* createShaderObject();
	virtual ITexture* createTexture();

	virtual IFrameBuffer* createFrameBuffer();

	virtual IPixelBuffer* createPixelBuffer();

	virtual IReFont* createFont();

	virtual IWindow* getMainWindow();

	virtual size_t getMemoryUsage() const;
	virtual size_t getMaxMemory() const;

	virtual void toNormalizedDeviceSpace(float &x, float &y);

private:

	void setScissorTest(bool enable);
	void setScissorRegion(int x, int y, int width, int height);

	void printInfo(GLinfo info);

	int stencilOpToGlInt(StencilOp op);
	int funcConstantToGlInt(FuncConstants funcConst);

	int counter;

	GLinfo info;

	RGB clearColor;
	FT_Library fontLibrary;

	GLWindow glWindow;

	DisplaySize size;

#if SUPPORT_VULKAN_WINDOW
	VKWindow vkWindow;
#endif

	RenderEngineCreateInfo reci;
	ScissorInfo scissorInfo;
};

#endif