#include "image_io2.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

namespace image_io {

torch::Tensor load_image(const std::string& file_path, std::function<torch::Tensor(torch::Tensor)> transform) {
	int width = 0;
	int height = 0;
	int depth = 0;
	std::unique_ptr<unsigned char, decltype(&stbi_image_free)> image_raw(stbi_load(file_path.c_str(), &width, &height, &depth, 0), &stbi_image_free);
	if (!image_raw) {
		throw std::runtime_error("Unable to load image file " + file_path + ".");
	}
	//throw std::invalid_argument(std::to_string(width) + "x" + std::to_string(height) + "x" + std::to_string(depth));

	return transform(torch::from_blob(image_raw.get(), {height, width, depth}, torch::kUInt8).clone().to(torch::kFloat32).permute({2, 0, 1}).div_(255));
}

void save_image(torch::Tensor tensor, const std::string& file_path, int64_t nrow, int64_t padding,
    bool normalize, const std::vector<double>& range,
    bool scale_each, torch::Scalar pad_value, ImageFormat format) {
}
}
