#include <iostream>
#include <sstream>
#include <string>

#include <png++/png.hpp>
#include <rlottie.h>

int main(int argc, char** argv) {

	// https://stackoverflow.com/a/18816712
	std::ostringstream anim_json_buf;
	anim_json_buf << std::cin.rdbuf();
	std::string anim_json = anim_json_buf.str();

	auto anim = rlottie::Animation::loadFromData(anim_json, "");
	size_t frames = anim->totalFrame();

	char pathbuf[128];
	size_t width = 512;
	size_t height = 512;
	auto* framebuf = new uint32_t[width * height];
	auto* pngs = new png::image<png::rgba_pixel>[frames];
	for (size_t frame = 0; frame < frames; frame++) {
		rlottie::Surface surface(framebuf, width, height, width * 4);
		anim->renderSync(frame, surface);
		// framebuf is now in BGRA format
		pngs[frame] = png::image<png::rgba_pixel>(width, height);
		auto& image = pngs[frame];
		for (size_t y = 0; y < height; y++) {
			for (size_t x = 0; x < width; x++) {
				uint32_t px = framebuf[y * width + x];
				uint8_t* c = reinterpret_cast<uint8_t*>(&px);
				image[y][x] = png::rgba_pixel(c[2], c[1], c[0], c[3]);
			}
		}
		snprintf(pathbuf, sizeof pathbuf, "out-tmp/%02d.png", frame);
		image.write(pathbuf);
	}
}
