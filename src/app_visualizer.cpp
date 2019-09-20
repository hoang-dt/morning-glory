#define ENTRY_CONFIG_IMPLEMENT_MAIN 1
#include <entry/entry.h>
#include <entry/entry.cpp>
#include <entry/entry_windows.cpp>
#include <imgui/imgui.h>
#include <common.h>
#include <bgfx_utils.h>
#include <stdio.h>
#include <math.h>
#include <bx/string.h>
#include <bx/timer.h>
#include <bimg/decode.h>
#include <entry/input.cpp>
#include <entry/cmd.cpp>
#include <nanovg/nanovg.h>
#include <bx/src/amalgamated.cpp>
#include <bgfx/src/amalgamated.cpp>
#include <nanovg/nanovg.cpp>
#include <nanovg/nanovg_bgfx.cpp>
#include <bimg/src/image.cpp>
#include <bimg/src/image_decode.cpp>
#include <bimg/src/image_gnf.cpp>
#define BLENDISH_IMPLEMENTATION
#include <blendish.h>
#include <meshoptimizer/src/vertexcodec.cpp>
#include <meshoptimizer/src/indexcodec.cpp>
#include "mg_common.h"
#include "mg_volume.h"
#include "mg_all.cpp"

namespace
{

using namespace mg;

#define ICON_SEARCH 0x1F50D
#define ICON_CIRCLED_CROSS 0x2716
#define ICON_CHEVRON_RIGHT 0xE75E
#define ICON_CHECK 0x2713
#define ICON_LOGIN 0xE740
#define ICON_TRASH 0xE729

// Returns 1 if col.rgba is 0.0f,0.0f,0.0f,0.0f, 0 otherwise
int isBlack( struct NVGcolor col )
{
	if( col.r == 0.0f && col.g == 0.0f && col.b == 0.0f && col.a == 0.0f )
	{
		return 1;
	}
	return 0;
}

static char* cpToUTF8(int cp, char* str)
{
	int n = 0;
	if (cp < 0x80) n = 1;
	else if (cp < 0x800) n = 2;
	else if (cp < 0x10000) n = 3;
	else if (cp < 0x200000) n = 4;
	else if (cp < 0x4000000) n = 5;
	else if (cp <= 0x7fffffff) n = 6;
	str[n] = '\0';
	switch (n)
	{
		case 6: str[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000; BX_FALLTHROUGH;
		case 5: str[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;  BX_FALLTHROUGH;
		case 4: str[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;   BX_FALLTHROUGH;
		case 3: str[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;     BX_FALLTHROUGH;
		case 2: str[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;      BX_FALLTHROUGH;
		case 1: str[0] = char(cp);                                          BX_FALLTHROUGH;
	}
	return str;
}

void DrawGrid(NVGcontext* Vg, const v2i& From, const v2i& Dims, const v2i& Strd) {
	nvgSave(Vg);
	for (int Y = From.Y; Y < From.Y + Dims.Y * Strd.Y; Y += Strd.Y) {
		for (int X = From.X; X < From.X + Dims.X * Strd.X; X += Strd.X) {
			nvgBeginPath(Vg);
			nvgRect(Vg, X, Y, Strd.X, Strd.Y);
			nvgStrokeColor(Vg, nvgRGBA(0,230,0,200) );
			nvgStroke(Vg);
		}
	}
	nvgRestore(Vg);
}

void DrawBox(NVGcontext* Vg, const v2i& From, const v2i& To) {
	nvgSave(Vg);
	nvgBeginPath(Vg);
	nvgRect(Vg, From.X, From.Y, To.X - From.X, To.Y - From.Y);
	nvgFillColor(Vg, nvgRGBA(230,0,0,30) );
	nvgFill(Vg);
	nvgRestore(Vg);
}

void drawWindow(struct NVGcontext* vg, const char* title, float x, float y, float w, float h)
{
	float cornerRadius = 3.0f;
	struct NVGpaint shadowPaint;
	struct NVGpaint headerPaint;

	nvgSave(vg);
	//	nvgClearState(vg);

	// Window
	nvgBeginPath(vg);
	nvgRect(vg, x,y, w,h);
	nvgFillColor(vg, nvgRGBA(28,30,34,192) );
	//	nvgFillColor(vg, nvgRGBA(0,0,0,128) );
	nvgFill(vg);

	// Header
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x+1,y+1, w-2,30, cornerRadius-1);
	nvgFillColor(vg, nvgRGBA(240,30,34,192));
	nvgFill(vg);
	nvgBeginPath(vg);
	// nvgMoveTo(vg, x+0.5f, y+0.5f+30);
	// nvgLineTo(vg, x+0.5f+w-1, y+0.5f+30);
	nvgStrokeColor(vg, nvgRGBA(0,0,0,32) );
	nvgStroke(vg);

	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_BOTTOM);

	// nvgFontBlur(vg,2);
	// nvgFillColor(vg, nvgRGBA(0,0,0,128) );
	// nvgText(vg, x+w/2,y+16+1, title, NULL);

	nvgFontBlur(vg,0);
	nvgFillColor(vg, nvgRGBA(220,220,220,160) );
	nvgText(vg, x+w/2,y+16, title, NULL);

	nvgRestore(vg);
}


struct DemoData
{
	int fontNormal, fontBold, fontIcons, fontEmoji;
	int images[12];
};

int32_t createImage(struct NVGcontext* _ctx, const char* _filePath, int _imageFlags)
{
	uint32_t size;
	void* data = load(_filePath, &size);
	if (NULL == data)
	{
		return 0;
	}

	bimg::ImageContainer* imageContainer = bimg::imageParse(
		  entry::getAllocator()
		, data
		, size
		, bimg::TextureFormat::RGBA8
		);
	unload(data);

	if (NULL == imageContainer)
	{
		return 0;
	}

	int32_t texId = nvgCreateImageRGBA(
		  _ctx
		, imageContainer->m_width
		, imageContainer->m_height
		, _imageFlags
		, (const uint8_t*)imageContainer->m_data
		);

	bimg::imageFree(imageContainer);

	return texId;
}

int32_t createFont(NVGcontext* _ctx, const char* _name, const char* _filePath)
{
	uint32_t size;
	void* data = load(_filePath, &size);
	if (NULL == data)
	{
		return -1;
	}

	return nvgCreateFontMem(_ctx, _name, (uint8_t*)data, size, 0);
}

int loadDemoData(struct NVGcontext* vg, struct DemoData* data)
{
	int i;

	if (vg == NULL)
		return -1;

	for (i = 0; i < 12; i++)
	{
		char file[128];
		bx::snprintf(file, 128, "images/image%d.jpg", i+1);
		data->images[i] = createImage(vg, file, 0);
		if (data->images[i] == 0)
		{
			bx::debugPrintf("Could not load %s.\n", file);
			return -1;
		}
	}

	int32_t result = 0;

	data->fontIcons = createFont(vg, "icons", "font/entypo.ttf");
	if (data->fontIcons == -1)
	{
		bx::debugPrintf("Could not add font icons.\n");
		result = -1;
	}

	data->fontNormal = createFont(vg, "sans", "font/roboto-regular.ttf");
	if (data->fontNormal == -1)
	{
		bx::debugPrintf("Could not add font italic.\n");
		result = -1;
	}

	data->fontBold = createFont(vg, "sans-bold", "font/roboto-bold.ttf");
	if (data->fontBold == -1)
	{
		bx::debugPrintf("Could not add font bold.\n");
		result = -1;
	}

	data->fontEmoji = createFont(vg, "emoji", "font/NotoEmoji-Regular.ttf");
	if (data->fontEmoji == -1)
	{
		bx::debugPrintf("Could not add font emoji.\n");
		result = -1;
	}

	nvgAddFallbackFontId(vg, data->fontNormal, data->fontEmoji);
	nvgAddFallbackFontId(vg, data->fontBold, data->fontEmoji);

	return result;
}

void freeDemoData(struct NVGcontext* vg, struct DemoData* data)
{
	int i;

	if (vg == NULL)
		return;

	for (i = 0; i < 12; i++)
		nvgDeleteImage(vg, data->images[i]);
}


void renderDemo(struct NVGcontext* vg, float mx, float my, float width, float height, float t, int blowup, struct DemoData* data)
{
	float x,y,popx,popy;

	x = width-520; y = height-420;
	//drawWindow(vg, "Widgets `n Stuff", x, y, 300, 400);
	DrawGrid(vg, v2i(100, 100), v2i(64, 64), v2i(8, 8));
	nvgRestore(vg);
}

class ExampleNanoVG : public entry::AppI
{
public:
	ExampleNanoVG(const char* _name, const char* _description)
		: entry::AppI(_name, _description)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_NONE;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type     = args.m_type;
		init.vendorId = args.m_pciId;
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		init.resolution.reset  = m_reset;
		bgfx::init(init);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
			);

		imguiCreate();

		m_nvg = nvgCreate(1, 0);
		bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

		loadDemoData(m_nvg, &m_data);

		bndSetFont(nvgCreateFont(m_nvg, "droidsans", "font/droidsans.ttf") );
		bndSetIconImage(createImage(m_nvg, "images/blender_icons16.png", 0) );

		m_timeOffset = bx::getHPCounter();
	}

	int shutdown() override
	{
		freeDemoData(m_nvg, &m_data);

		nvgDelete(m_nvg);

		imguiDestroy();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
			imguiBeginFrame(m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			showExampleDialog(this);
			if (!ImGui::GetIO().WantCaptureMouse) {
				if (m_mouseReleased && m_mouseState.m_buttons[entry::MouseButton::Left]) {
					m_mouseDown = v2i(m_mouseState.m_mx, m_mouseState.m_my);
					m_mouseReleased = false;
				}
				if (m_mouseState.m_buttons[entry::MouseButton::Left]) {
					m_mouseUp = v2i(m_mouseState.m_mx, m_mouseState.m_my);
				} else {
					m_mouseReleased = true;
				}
			}
			imguiEndFrame();

			int64_t now = bx::getHPCounter();
			const double freq = double(bx::getHPFrequency() );
			float time = (float)( (now-m_timeOffset)/freq);

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			nvgBeginFrame(m_nvg, m_width, m_height, 1.0f);

			renderDemo(m_nvg, float(m_mouseState.m_mx), float(m_mouseState.m_my), float(m_width), float(m_height), time, 0, &m_data);
			DrawBox(m_nvg, m_mouseDown, m_mouseUp); // selection

			nvgEndFrame(m_nvg);

			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;

	entry::MouseState m_mouseState;
	v2i m_mouseDown = v2i::Zero;
	v2i m_mouseUp = v2i::Zero;
	bool m_mouseReleased = true;

	int64_t m_timeOffset;

	NVGcontext* m_nvg;
	DemoData m_data;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(ExampleNanoVG, "20-nanovg", "NanoVG is small antialiased vector graphics rendering library.");
