#include <torch/script.h>
#include <torch/torch.h>
#include <iostream>
#include <memory>

#include "image_io.h"
#include "stb_image_write.h"

void save_img3__(const torch::Tensor &fgr0, const torch::Tensor &pha0) {
		auto fgr = fgr0.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
		auto pha = pha0.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
		size_t width = fgr.size(1);
		size_t height = fgr.size(0);
		unsigned char *pf = fgr.data_ptr<unsigned char>();
		unsigned char *p  = pha.data_ptr<unsigned char>();
		unsigned char min = (unsigned char)(255 * 0.76);
		std::cout << width << "x" << height << " " << (int)min << std::endl;

		size_t min_j = 0, max_j = 0, min_i = height, max_i = 0;
		bool can_min_j = true;

		for (size_t j = 0; j < height; j++){
			for (size_t i = 0; i < width; i++){
				unsigned char a = p[j * width + i + 0];
				unsigned char b = p[j * width + i + 1];
				unsigned char c = p[j * width + i + 2];
				if(a <= min && b <= min && c <= min) {
					if(can_min_j && min_j < j) min_j = j;
				} else {
					can_min_j = false;
					if(min_i > i) min_i = i;
					max_j = j;
					if(max_i < i) max_i = i;
				}
			}
		}

		size_t w = max_i-min_i, h = max_j-min_j;
		size_t len = (w + 1) * (h + 1) * 4;
		std::cout << "  积极剪裁 " << min_i << "," << min_j << "," << (w) << "," << (h) << " len" << len << std::endl;
		unsigned char* p2 = new unsigned char[len];
		if(!p2) {
			std::cout << "啊" << std::endl;
			return;
		}
		for (size_t j = min_j; j <= max_j; j++){
			for (size_t i = min_i; i <= max_i; i++){
				unsigned char a = p[j * width + i + 0];
				unsigned char b = p[j * width + i + 1];
				unsigned char c = p[j * width + i + 2];
				size_t o = ((j - min_j) * w + (i - min_i)) * 4;
				//std::cout << "\r\n" << j << "," << i << " " << (int)a << "," << (int)b << "," << (int)c << " ";
				if(a <= min && b <= min && c <= min) {
					for(size_t k = 0; k < 4; k++)
						p2[o + k] = 0;
				} else {
					for(size_t k = 0; k < 3; k++)
						p2[o + k] = pf[j * width * 3 + i * 3 + k];
					p2[o + 3] = 255;
				}
			}
		}

		stbi_write_png("/tmp/out.png",  width, height, 3, pf, 4800);
		stbi_write_png("/tmp/out2.png", width, height, 1, p,  1600);
		stbi_write_png("/tmp/out4.png", w, h, 4, p2, 0);
		delete p2;
}

int main() {

	torch::DeviceType device_type = torch::kCPU;
	torch::Device device(device_type);
	
	auto imgpath = "/_pic/美女/真人/1/49.jpg";
	std::cout << imgpath << std::endl;
	torch::data::transforms::Normalize<> normalize_transform({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
	auto content = image_io::load_image(imgpath, {}, normalize_transform)
		.unsqueeze_(0)
		.contiguous()
		.to(device);

	auto mo = torch::jit::load("/opt2/src/mk----/RobustVideoMatting/model/rvm_resnet50_fp32.torchscript", device);

	c10::optional<torch::Tensor> tensorRec0;
	c10::optional<torch::Tensor> tensorRec1;
	c10::optional<torch::Tensor> tensorRec2;
	c10::optional<torch::Tensor> tensorRec3;

	std::vector<torch::jit::IValue> v;
	v.push_back(content);
	v.push_back(tensorRec0);
	v.push_back(tensorRec1);
	v.push_back(tensorRec2);
	v.push_back(tensorRec3);
	v.push_back(0.4);
	auto outputs = mo.forward(v).toList();

	const auto &fgr = outputs.get(0).toTensor();
	const auto &pha = outputs.get(1).toTensor();
	tensorRec0 = outputs.get(2).toTensor();
	tensorRec1 = outputs.get(3).toTensor();
	tensorRec2 = outputs.get(4).toTensor();
	tensorRec3 = outputs.get(5).toTensor();

	save_img3__(fgr, pha);
}
