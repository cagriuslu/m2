#include <m2/sdl/FontGenerator.h>
#include <m2/Game.h>
#include <m2/FileSystem.h>
#include <m2/Log.h>

TTF_Font* m2::sdl::FontGenerator::operator()(int size) {
	LOG_DEBUG("Reopening the default font with size", size);
	auto path = resource_path() / M2G_PROXY.default_font_path;
	auto* font = TTF_OpenFont(path.c_str(), size);
	if (not font) {
		throw M2_ERROR("Unable to open font: " + path.string() + " error: " + TTF_GetError());
	}
	return font;
}
