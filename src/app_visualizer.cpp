#define ENTRY_CONFIG_IMPLEMENT_MAIN 1
#define USE_ENTRY 1
#include <entry/entry.h>
#include <entry/entry.cpp>
#include <entry/entry_windows.cpp>
#include <entry/input.cpp>
#include <entry/cmd.cpp>
#include <imgui/imgui.h>
#include <common.h>
#include <bgfx_utils.h>
#include <stdio.h>
#include <math.h>
#include <bx/string.h>
#include <bx/timer.h>
#include <bimg/decode.h>
#include <nanovg/nanovg.h>
#include <nanovg/nanovg.cpp>
#include <nanovg/nanovg_bgfx.cpp>
#define BLENDISH_IMPLEMENTATION
#include <blendish.h>
#include <meshoptimizer/src/vertexcodec.cpp>
#include <meshoptimizer/src/indexcodec.cpp>
#include "mg_common.h"
#include "mg_math.h"
#include "mg_rendering.h"
#include "mg_volume.h"
#include "mg_wavelet.h"
#include "mg_all.cpp"
#define NOC_FILE_DIALOG_WIN32
#define NOC_FILE_DIALOG_IMPLEMENTATION
#include "noc_file_dialog.h"

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

void DrawSubbandSep(NVGcontext* Vg, const v2i& TopLeft, const v2i& N, const v2i& Spacing, int NLevels) {
	nvgSave(Vg);
	nvgStrokeColor(Vg, nvgRGBA(100,0,0,250));
  nvgStrokeWidth(Vg, 2);
  v2i M = N;
  for (int L = 0; L < NLevels; ++L) {
    M = (M + 1) / 2;
    nvgBeginPath(Vg);
    nvgMoveTo(Vg, TopLeft.X + Spacing.X * M.X - Spacing.X / 2, TopLeft.Y - Spacing.X / 2);
    nvgLineTo(Vg, TopLeft.X + Spacing.X * M.X - Spacing.X / 2, TopLeft.Y + Spacing.Y * M.Y * 2 - Spacing.Y / 2);
    nvgStroke(Vg);
    nvgBeginPath(Vg);
    nvgMoveTo(Vg, TopLeft.X - Spacing.X / 2, TopLeft.Y + M.Y * Spacing.Y - Spacing.Y / 2);
    nvgLineTo(Vg, TopLeft.X + Spacing.X * M.X * 2 - Spacing.X / 2, TopLeft.Y + M.Y * Spacing.Y - Spacing.Y / 2);
    nvgStroke(Vg);
  }
	nvgRestore(Vg);
}

enum draw_mode { Fill, Stroke, FillStroke };
void DrawGrid(NVGcontext* Vg, const v2i& From, const v2i& Dims, const v2i& Spacing, const NVGcolor& Color, draw_mode Mode) {
	nvgSave(Vg);
  nvgStrokeWidth(Vg,1.0f);
  if (Mode != Stroke)
    nvgFillColor(Vg, Color);
  if (Mode != Fill)
    nvgStrokeColor(Vg, Color);
	for (int Y = From.Y; Y < From.Y + Dims.Y * Spacing.Y; Y += Spacing.Y) {
		for (int X = From.X; X < From.X + Dims.X * Spacing.X; X += Spacing.X) {
			nvgBeginPath(Vg);
			//nvgCircle(Vg, X, Y, 4);
      nvgRect(Vg, X - 4, Y - 4, 8, 8);
      //nvgStrokeColor(Vg, nvgRGBA(0,130,0,200) );
      if (Mode != Fill)
        nvgStroke(Vg);
      if (Mode != Stroke)
			  nvgFill(Vg);
		}
	}
	nvgRestore(Vg);
}

void DrawGrid(NVGcontext* Vg, const v2i& From, const v2i& Dims, const v2i& Spacing, const array<v3<u8>>& VolColor, draw_mode Mode) {
	nvgSave(Vg);
  nvgStrokeWidth(Vg,1.0f);
	for (int Y = From.Y; Y < From.Y + Dims.Y * Spacing.Y; Y += Spacing.Y) {
		for (int X = From.X; X < From.X + Dims.X * Spacing.X; X += Spacing.X) {
      const v3<u8>& Color = VolColor[(Y - From.Y) / Spacing.Y * Dims.X + (X - From.X) / Spacing.X];
      if (Mode != Stroke)
        nvgFillColor(Vg, nvgRGBA(Color.R, Color.G, Color.B, 255));
      if (Mode != Fill)
        nvgStrokeColor(Vg, nvgRGBA(Color.R, Color.G, Color.B, 255));
			nvgBeginPath(Vg);
      //printf("%d %d %d\n", Color.R, Color.G, Color.B);
			//nvgCircle(Vg, X, Y, 4);
      nvgRect(Vg, X - 4, Y - 4, 8, 8);
      //nvgStrokeColor(Vg, nvgRGBA(0,130,0,200) );
      if (Mode != Fill)
        nvgStroke(Vg);
      if (Mode != Stroke)
			  nvgFill(Vg);
		}
	}
	nvgRestore(Vg);
}

void DrawBox(NVGcontext* Vg, const v2i& From, const v2i& To, const NVGcolor& Color) {
	nvgSave(Vg);
	nvgBeginPath(Vg);
	nvgRect(Vg, From.X, From.Y, To.X - From.X, To.Y - From.Y);
	nvgFillColor(Vg, Color);
	nvgFill(Vg);
	nvgRestore(Vg);
}

void DrawText(NVGcontext* Vg, const v2i& Where, cstr Text, int Size) {
  nvgSave(Vg);
	nvgFillColor(Vg, nvgRGBA(58,30,34,250));
  nvgFontSize(Vg, Size);
  nvgFontFace(Vg, "sans");
  nvgTextAlign(Vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
	nvgFontBlur(Vg, 0);
  nvgText(Vg, Where.X, Where.Y, Text, nullptr);
  nvgRestore(Vg);
}

struct box {
  v2i From;
  v2i To;
};
box GetPointBox(const v2i& MouseDown, const v2i& MouseUp, const v2i& TopLeft,
  const v2i& N, const v2i& Spacing)
{
  v2i MouseFrom = Max(Min(MouseDown, MouseUp), TopLeft);
  v2i MouseTo = Min(Max(MouseDown, MouseUp), TopLeft + N * Spacing);
  int PointFromX = (int)ceil(float(MouseFrom.X - TopLeft.X) / Spacing.X);
  int PointFromY = (int)ceil(float(MouseFrom.Y - TopLeft.Y) / Spacing.Y);
  int PointToX = (int)floor(float(MouseTo.X - TopLeft.X) / Spacing.X);
  int PointToY = (int)floor(float(MouseTo.Y - TopLeft.Y) / Spacing.Y);
  return box{ v2i(PointFromX, PointFromY), v2i(PointToX, PointToY) };
}

void drawWindow(struct NVGcontext* vg, const char* title, float x, float y, float w, float h)
{
	float cornerRadius = 3.0f;
	struct NVGpaint shadowPaint;
	struct NVGpaint headerPaint;

	nvgSave(vg);
	//fonsClearState(vg);

	// Window
	//nvgBeginPath(vg);
	//nvgRect(vg, x,y, w,h);
	//nvgFillColor(vg, nvgRGBA(28,30,34,192) );
	//nvgFillColor(vg, nvgRGBA(0,0,0,128) );
	//nvgFill(vg);

	// Header
	//nvgBeginPath(vg);
	//nvgRoundedRect(vg, x+1,y+1, w-2,30, cornerRadius-1);
	//nvgFillColor(vg, nvgRGBA(240,30,34,192));
	//nvgFill(vg);
	//nvgBeginPath(vg);
	//nvgMoveTo(vg, x+0.5f, y+0.5f+30);
	//nvgLineTo(vg, x+0.5f+w-1, y+0.5f+30);
	//nvgStrokeColor(vg, nvgRGBA(0,0,0,32) );
	//nvgStroke(vg);

	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_BOTTOM);

	//nvgFontBlur(vg,2);
	//nvgFillColor(vg, nvgRGBA(0,0,0,128) );
	//nvgText(vg, x+w/2,y+16+1, title, NULL);

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
	drawWindow(vg, "Widgets `n Stuff", x, y, 300, 400);
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
			, 0xffffffff
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

    BuildSubbands(v3i(N, 1), NLevels, &Subbands);
    BuildSubbands(v3i(N, 1), NLevels, &SubbandsG);
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
      const uint8_t* data = inputGetChar();
      const uint8_t character = data != nullptr ? 0u[data] : 0u;
			imguiBeginFrame(m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				, character);

			//showExampleDialog(this);
			static char buf1[8] = "33"; ImGui::InputText("Nx", buf1, 8, ImGuiInputTextFlags_CharsDecimal);
			static char buf2[8] = "33"; ImGui::InputText("Ny", buf2, 8, ImGuiInputTextFlags_CharsDecimal);
			if (ImGui::Button("Draw grid")) {
				ToInt(buf1, &N.X);
				ToInt(buf2, &N.Y);
			}
			if (!ImGui::GetIO().WantCaptureMouse) {
        int ValDomainBot = ValDomainTopLeft.Y + N.Y * Spacing.Y;
				if (m_mouseReleased && m_mouseState.m_buttons[entry::MouseButton::Left]) {
          if (m_mouseState.m_my < WavDomainTopLeft.Y - (WavDomainTopLeft.Y - ValDomainBot) / 2)
            ValMouseDown = v2i(m_mouseState.m_mx, m_mouseState.m_my);
          else
            WavMouseDown = v2i(m_mouseState.m_mx, m_mouseState.m_my);
					m_mouseReleased = false;
				}
				if (m_mouseState.m_buttons[entry::MouseButton::Left]) {
          if (m_mouseState.m_my < WavDomainTopLeft.Y - (WavDomainTopLeft.Y - ValDomainBot) / 2)
					  ValMouseUp = v2i(m_mouseState.m_mx, m_mouseState.m_my);
          else
					  WavMouseUp = v2i(m_mouseState.m_mx, m_mouseState.m_my);
				} else {
					m_mouseReleased = true;
				}
			}
			if (ImGui::SliderInt("Number of levels", &NLevels, 1, 4)) {
        BuildSubbands(v3i(N, 1), NLevels, &Subbands);
        BuildSubbands(v3i(N, 1), NLevels, &SubbandsG);
      }

			int64_t now = bx::getHPCounter();
			const double freq = double(bx::getHPFrequency() );
			float time = (float)( (now-m_timeOffset)/freq);

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			nvgBeginFrame(m_nvg, m_width, m_height, 1.0f);

			//renderDemo(m_nvg, float(m_mouseState.m_mx), float(m_mouseState.m_my), float(m_width), float(m_height), time, 0, &m_data);
			//DrawBox(m_nvg, ValMouseDown, ValMouseUp); // selection
			//DrawBox(m_nvg, WavMouseDown, WavMouseUp); // selection
      // draw the val grid
      if (!Vol.Buffer)
			  DrawGrid(m_nvg, ValDomainTopLeft, N, Spacing, nvgRGBA(0, 130, 0, 200), draw_mode::Stroke);
      else
			  DrawGrid(m_nvg, ValDomainTopLeft, N, Spacing, VolColor, draw_mode::Fill);

			DrawGrid(m_nvg, WavDomainTopLeft, N, Spacing, nvgRGBA(0, 130, 0, 200), draw_mode::Stroke);
      DrawSubbandSep(m_nvg, WavDomainTopLeft, N, Spacing, NLevels);
      // TODO: select the wavelet grid
      // TODO: draw the wavgrid, wrkgrid, valgrid
      // TODO: select between wrkgrid and valgrid with a radio button
      /* print the selection in the Val domain */
      box ValBox = GetPointBox(ValMouseDown, ValMouseUp, ValDomainTopLeft, N, Spacing);
      char Temp[50];
      sprintf(Temp, "(%d %d) to (%d %d)", ValBox.From.X, ValBox.From.Y, ValBox.To.X, ValBox.To.Y);
      DrawText(m_nvg, v2i(50, 30), Temp, 20);
      //DrawGrid(m_nvg, ValDomainTopLeft + ValBox.From * Spacing, ValBox.To - ValBox.From + 1, Spacing, nvgRGBA(130, 0, 0, 200), draw_mode::Fill);
      DrawBox(m_nvg, ValDomainTopLeft + ValBox.From * Spacing - Spacing / 2, ValDomainTopLeft + ValBox.From * Spacing + (ValBox.To - ValBox.From) * Spacing + Spacing / 2, nvgRGBA(230, 0, 0, 50));

      /* Wav domain */
      box WavBox = GetPointBox(WavMouseDown, WavMouseUp, WavDomainTopLeft, N, Spacing);
      //sprintf(Temp, "(%d %d) to (%d %d)", WavMouseDown.X, WavMouseDown.Y, WavMouseUp.X, WavMouseUp.Y);
      //DrawText(m_nvg, v2i(50, 530), Temp, 20);
      int WavFromX = (int)ceil(float(Max(0, Min(WavMouseDown.X, WavDomainTopLeft.X + N.X * Spacing.X)) - WavDomainTopLeft.X) / Spacing.X);
      int WavFromY = (int)ceil(float(Max(0, Min(WavMouseDown.Y, WavDomainTopLeft.Y + N.Y * Spacing.Y)) - WavDomainTopLeft.Y) / Spacing.Y);
      int Sb = -1;
      for (int I = 0; I < Size(Subbands); ++I) {
        if (v3i(WavFromX, WavFromY, 0) >= From(Subbands[I]) && v3i(WavFromX, WavFromY, 0) <= To(Subbands[I])) {
          Sb = I;
          break;
        }
      }
      extent WavCrop = /*extent(v3i(WavBox.From, 0), v3i(WavBox.To - WavBox.From + 1, 1)); */Crop(extent(v3i(WavBox.From, 0), v3i(WavBox.To - WavBox.From + 1, 1)), Subbands[Sb]);
      DrawGrid(m_nvg, WavDomainTopLeft + From(WavCrop).XY * Spacing, (To(WavCrop) - From(WavCrop)).XY, Spacing, nvgRGBA(130, 0, 0, 200), draw_mode::Fill);
      sprintf(Temp, "(%d %d) to (%d %d)", From(WavCrop).X, From(WavCrop).Y, To(WavCrop).X, To(WavCrop).Y);
      DrawText(m_nvg, v2i(50, 450), Temp, 20);
      sprintf(Temp, "Subband %d", Sb);
      DrawText(m_nvg, v2i(50, 470), Temp, 20);

      /* WavGrid domain */
      //DrawGrid(m_nvg, WavGDomainTopLeft + ValBox.From * Spacing, ValBox.To - ValBox.From, Spacing, nvgRGBA(130, 0, 0, 200));
			DrawGrid(m_nvg, WavGDomainTopLeft, N, Spacing, nvgRGBA(0, 130, 0, 200), draw_mode::Stroke);
      extent WavCropLocal = WavCrop;
      SetFrom(&WavCropLocal, From(WavCrop) - From(Subbands[Sb]));
      grid WavGGrid = SubGrid(SubbandsG[Sb], WavCropLocal);
      sprintf(Temp, "from (%d %d) dims (%d %d) stride (%d %d)", From(WavGGrid).X, From(WavGGrid).Y, Dims(WavGGrid).X, Dims(WavGGrid).Y, Strd(WavGGrid).X, Strd(WavGGrid).Y);
      DrawText(m_nvg, v2i(500, 30), Temp, 20);
      //box WavGBox = GetPointBox(0, WavGFrom(WavGGrid).XY, WavGDomainTopLeft, N, Spacing);
			DrawGrid(m_nvg, WavGDomainTopLeft + From(WavGGrid).XY * Spacing, Dims(WavGGrid).XY, Spacing * Strd(WavGGrid).XY, nvgRGBA(0, 130, 0, 200), draw_mode::Fill);
      //DrawBox(m_nvg, WavGDomainTopLeft + From(WavGGrid).XY * Spacing - Spacing / 2, WavGDomainTopLeft + From(WavGGrid).XY * Spacing + (Dims(WavGGrid).XY - 1) * Spacing * Strd(WavGGrid).XY - Spacing / 2, nvgRGBA(0, 230, 0, 50));
      DrawBox(m_nvg, WavGDomainTopLeft + ValBox.From * Spacing - Spacing / 2, WavGDomainTopLeft + ValBox.From * Spacing + (ValBox.To - ValBox.From) * Spacing + Spacing / 2, nvgRGBA(230, 0, 0, 50));
      extent Footprint = WavFootprint(2, Sb, WavGGrid);
      DrawBox(m_nvg, WavGDomainTopLeft + From(Footprint).XY * Spacing - Spacing / 2, WavGDomainTopLeft + From(Footprint).XY * Spacing + (Dims(Footprint) - 1).XY * Spacing + Spacing / 2, nvgRGBA(0, 0, 230, 50));
      // TODO: highlight the output WavGrid
      extent ValExt(v3i(ValBox.From, 0), v3i(ValBox.To - ValBox.From + 1, 1));
      wav_grids WavGrids = ComputeWavGrids(2, Sb, ValExt, WavGGrid, v3i(1000));
      sprintf(Temp, "from (%d %d) dims (%d %d) stride (%d %d)", From(WavGrids.WavGrid).X, From(WavGrids.WavGrid).Y, Dims(WavGrids.WavGrid).X, Dims(WavGrids.WavGrid).Y, Strd(WavGrids.WavGrid).X, Strd(WavGrids.WavGrid).Y);
      DrawText(m_nvg, v2i(500, 50), Temp, 20);
      ImGui::Checkbox("WavGrid", &DrawWavGrid);
      if (DrawWavGrid)
			  DrawGrid(m_nvg, WavGDomainTopLeft + From(WavGrids.WavGrid).XY * Spacing, Dims(WavGrids.WavGrid).XY, Spacing * Strd(WavGrids.WavGrid).XY, nvgRGBA(130, 0, 130, 200), draw_mode::Fill);
      else
			  DrawGrid(m_nvg, WavGDomainTopLeft + From(WavGrids.WrkGrid).XY * Spacing, Dims(WavGrids.WrkGrid).XY, Spacing * Strd(WavGrids.WrkGrid).XY, nvgRGBA(130, 130, 0, 200), draw_mode::Fill);
      // draw the val grid
      DrawGrid(m_nvg, ValDomainTopLeft + From(WavGrids.ValGrid).XY * Spacing, Dims(WavGrids.ValGrid).XY, Spacing * Strd(WavGrids.ValGrid).XY, nvgRGBA(130, 0, 0, 200), draw_mode::Fill);
      if (ImGui::Button("Load transfer function")) {
        cstr TfFile = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "json\0*.json", nullptr, nullptr);
        if (TfFile) {
          printf("Loading transfer function %s\n", TfFile);
          Dealloc(&Tf);
          auto Result = ReadTransferFunc(TfFile, &Tf);
          if (!Result) {
            printf("Error: %s\n", ToString(Result));
          }
        }
      }
      if (ImGui::Button("Load data")) {
        cstr DataFile = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "raw\0*.raw", nullptr, nullptr);
        if (DataFile) {
          printf("Loading raw file %s\n", DataFile);
          Dealloc(&Vol);
          auto Result = ReadVolume(DataFile, v3i(N, 1), dtype::float64, &Vol);
          if (!Result) {
            printf("Error: %s\n", ToString(Result));
          } else if (Size(Vol.Buffer) != Prod(N) * SizeOf(dtype::float64)) {
            printf("Error: size mismatch\n");
            Dealloc(&Vol);
          } else { // succeed
            // find the min, max value of Volume
            auto MM = MinMaxElem(Begin<f64>(Vol), End<f64>(Vol));
            MinVal = *(MM.Min), MaxVal = *(MM.Max);
            Resize(&VolColor, Prod(N));
            for (int Y = 0; Y < N.Y; ++Y) {
              for (int X = 0; X < N.X; ++X) {
                f64 Val = (Vol.At<f64>(Y * N.X + X) - MinVal) / (MaxVal - MinVal);
                VolColor[Y * N.X + X] = v3<u8>(GetRGB(Tf, Val) * 255.0);
              }
            }
          }
        }
      }

			imguiEndFrame();
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
	v2i ValMouseDown = v2i::Zero;
	v2i ValMouseUp = v2i::Zero;
	v2i WavMouseDown = v2i::Zero;
	v2i WavMouseUp = v2i::Zero;
	bool m_mouseReleased = true;
  bool DrawWavGrid = true;
  transfer_func Tf;
  volume Vol;
  f64 MinVal, MaxVal;
  array<v3<u8>> VolColor;

	int64_t m_timeOffset;

	NVGcontext* m_nvg;
	DemoData m_data;
  v2i ValDomainTopLeft = v2i(50, 50);
  v2i WavDomainTopLeft = v2i(50, 500);
  v2i WavGDomainTopLeft = v2i(500, 50);
	v2i N = v2i(33, 33); // total dimensions
  v2i Spacing = v2i(12, 12);
	int NLevels = 1;
  array<extent> Subbands;
  array<grid> SubbandsG;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(ExampleNanoVG, "20-nanovg", "NanoVG is small antialiased vector graphics rendering library.");
