// SPDX-License-Identifier: BlueOak-1.0.0

#pragma once

#include <cstdint>
#include <limits>
#include <cmath>
#include <vector>
#include <string>
#include <utility>

#include <rlottie.h>
#include <apngasm.h>
// we only need to *write* APNGs
#undef APNG_READ_SUPPORTED
#undef APNG_SPECS_SUPPORTED

namespace tgs2apng {
	// Given a lottie animation, render a buffer of APNG data.
	// Return success/fail
	bool render(
		const std::string& lottie_data,
		const std::string& output_path,
		size_t width,
		size_t height
	);
	namespace internal {
		std::pair<uint16_t, uint16_t> fps_to_frame_delay(double fps);
		std::pair<int64_t, int64_t> as_integer_ratio(double d);
	}
}
