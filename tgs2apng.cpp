// SPDX-License-Identifier: BlueOak-1.0.0

#include <cstdlib>  // for exit status macros
#include <cstdio>  // for fprintf and friends
// for reading from stdin
#include <iostream>
#include <sstream>

#include "tgs2apng.hpp"

bool tgs2apng::render(
	const std::string& lottie_data,
	const std::string& output_path,
	size_t width,
	size_t height
) {
	auto anim = rlottie::Animation::loadFromData(lottie_data, "");
	if (!anim) return false;
	size_t frames = anim->totalFrame();
	double fps = anim->frameRate();
	auto frame_delay = tgs2apng::internal::fps_to_frame_delay(fps);

	// this struct defines the output format of rlottie::Animation::render()
	struct bgra { uint8_t b, g, r, a; };
	auto framebuf_bgra = std::vector<struct bgra>(width * height);
	// XXX figure out how to rewrite the bgra vector in-place
	auto framebuf_rgba = std::vector<apngasm::rgba>(width * height);
	auto apng = new apngasm::APNGAsm();

	std::fprintf(stderr, "Frame count: %d\n", frames);
	std::fprintf(stderr, "Frame delay: %d/%d\n", frame_delay.first, frame_delay.second);

	for (size_t frame = 0; frame < frames; frame++) {
		rlottie::Surface surface(reinterpret_cast<uint32_t*>(framebuf_bgra.data()), width, height, width * 4);
		anim->renderSync(frame, surface);
		// rewrite the framebuf to RGBA
		for (size_t y = 0; y < height; y++) {
			for (size_t x = 0; x < width; x++) {
				auto px = framebuf_bgra[y * height + x];
				framebuf_rgba[y * height + x] = apngasm::rgba {px.r, px.g, px.b, px.a};
			}
		}
		apng->addFrame(apngasm::APNGFrame(
			framebuf_rgba.data(),
			width, height,
			frame_delay.first, frame_delay.second
		));
	}
	bool rv = apng->assemble(output_path);
	delete apng;
	return rv;
}

std::pair<uint16_t, uint16_t> tgs2apng::internal::fps_to_frame_delay(double fps) {
	auto ratio = tgs2apng::internal::as_integer_ratio(fps);
	// delay = 1/framerate
	auto d = ratio.first;
	auto n = ratio.second;
	while (d > std::numeric_limits<uint16_t>::max()) {
		n >>= 1;
		d >>= 1;
	}
	return std::make_pair(n, d);
}

std::pair<int64_t, int64_t> tgs2apng::internal::as_integer_ratio(double d) {
	// based on float.as_integer_ratio from the CPython source
	// https://github.com/python/cpython/blob/v3.9.1/Objects/floatobject.c#L1494-L1581
	// SPDX-License-Identifier: PSF-2.0
	double float_part;
	int exponent;
	float_part = std::frexp(d, &exponent);
	for (int i = 0; i < 300 && float_part != std::floor(float_part); i++) {
		float_part *= 2.0;
		exponent--;
	}
	auto numerator = static_cast<int64_t>(float_part);
	int64_t denominator = 1;
	if (exponent > 0) {
		numerator <<= exponent;
	} else {
		denominator <<= exponent;
	}
	return std::make_pair(numerator, denominator);
}


int main(int argc, char** argv) {
	if (argc != 2) {
		std::fprintf(stderr, "Usage: %s <output path>.png < anim.json\n", argv[0]);
		return EXIT_FAILURE;
	}
	// https://stackoverflow.com/a/18816712
	std::ostringstream anim_json_buf;
	anim_json_buf << std::cin.rdbuf();
	std::string anim_json = anim_json_buf.str();
	return tgs2apng::render(anim_json, argv[1], 256, 256) ? EXIT_SUCCESS : EXIT_FAILURE;
}
