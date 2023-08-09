#include <torch/script.h>
#include <torch/torch.h>
#include <iostream>
#include <memory>

#include "image_io.h"

int main() {

	torch::DeviceType device_type = torch::kCPU;
	/*if (torch::cuda::is_available()) {
		device_type = torch::kCUDA;
		std::cout << "Running on a GPU" << std::endl;
	} else {
		std::cout << "Running on a CPU" << std::endl;
	}*/
	torch::Device device(device_type);
	
	torch::data::transforms::Normalize<> normalize_transform({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
	torch::data::transforms::Normalize<> denormalize_transform({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
	
	auto imgpath =
		"/opt2/src/github.com/pytorch/examples/fast_neural_style/images/content-images/amber.jpg"
		;
	std::cout << imgpath << std::endl;
	auto content = image_io::load_image(imgpath, /*300*/{}, normalize_transform)
		
		/*//.permute({2,0,1})
		.toType(torch::kFloat)
		.div(255)*/
		
		.unsqueeze_(0)
		.to(device)
		;
	//亮度-对比度 127 127
	{
		auto image = normalize_transform(content.clone());
		image_io::save_image(image, "/tmp/out2.png", 1, 0);
	}
	{
		auto image = denormalize_transform(content.clone());
		image_io::save_image(image, "/tmp/out3.png", 1, 0);
	}

	//const std::string modelNameMosaic = "mosaic_cpp.pt";
	auto mo/*duleCandy*/ = torch::jit::load(/*modelNameCandy*/"saved_models/candy.pt", device);
	mo.eval();
	/*for (auto p : mo.parameters()) {
		std::cout << p.device() << std::endl;
	}*/

	std::vector<torch::jit::IValue> inputs;
	inputs.push_back(content);
	at::Tensor output = mo.forward(inputs).toTensor();
	//std::cout << output << std::endl;

	auto image = denormalize_transform(output[0].to(torch::kCPU).clone()
			//.squeeze(0)
		)
		.clamp_(0, 255)
		//.transpose_(1, 2) //旋转
		;
	image_io::save_image(image, "/tmp/out.png", 1, 0, true);
	image_io::save_image(image, "/tmp/out4.png");
	image_io::save_image(image, "/tmp/out5.png", 1, 0, true, {}, true);
	image_io::save_image(image, "/tmp/out6.png", 1, 0, true, {}, true, 10);
}

/*
	std::vector<torch::jit::IValue> inputs;
	inputs.push_back(torch::ones({1, 3, 224, 224}));

	at::Tensor output = m->h_.forward(inputs).toTensor();
*/
	//std::cout << output.slice(/*dim=*/1, /*start=*/0, /*end=*/5) << '\n';
