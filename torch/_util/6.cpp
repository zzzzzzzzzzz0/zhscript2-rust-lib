#include <torch/script.h>
#include <torch/torch.h>
#include <iostream>
#include <memory>

#include "image_io.h"

int main() {
	std::vector<std::string> classnames;
	std::ifstream f("coco.names");
	std::string name = "";
	while (std::getline(f, name))
	{
		classnames.push_back(name);
	}
	std::cout << classnames.size() << std::endl;
	
	torch::DeviceType device_type = torch::kCPU;
	torch::Device device(device_type);

	auto mopath =
		//"/opt2/opt/yolov5/yolov5s.torchscript.pt"
		"yolov5s.torchscript.pt"
		;
	std::cout << mopath << std::endl;
	auto mo = torch::jit::load(mopath, device);
	mo.eval();

	torch::data::transforms::Normalize<> normalize_transform({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
	torch::data::transforms::Normalize<> denormalize_transform({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
	
	auto imgpath = "./zidane.jpg";
	std::cout << imgpath << std::endl;
	auto content = image_io::load_image(imgpath, 300/*{}*/, normalize_transform)
		
		.permute({2,0,1})
		.toType(torch::kFloat)
		.div(255)
		
		.unsqueeze_(0)
		.to(device)
		;

	at::Tensor output = mo.forward({content})
		.toTuple()->elements()[0]
		.toTensor();
	std::cout << output << std::endl;

	/*auto image = denormalize_transform(output[0].to(torch::kCPU).clone()
			//.squeeze(0)
		)
		.clamp_(0, 255)
		//.transpose_(1, 2) //旋转
		;
	image_io::save_image(image, "/tmp/out.png", 1, 0, true);*/
}
