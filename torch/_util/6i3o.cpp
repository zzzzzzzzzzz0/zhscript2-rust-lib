#include <torch/script.h>
#include <torch/torch.h>
#include <iostream>
#include <memory>

#include "image_io.h"

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

	auto tensorTargetBgr = torch::tensor({0.47,1.,0.6,}).toType(torch::kFloat).to(device).view({1, 3, 1, 1});
	auto res_tensor = pha * fgr + (1 - pha) * tensorTargetBgr;

	torch::data::transforms::Normalize<> denormalize_transform({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
	auto image = denormalize_transform(res_tensor.to(torch::kCPU).clone())
		.clamp_(0, 255)
		;
	image_io::save_image(image, "/tmp/out.png", 1, 0, true);
}
