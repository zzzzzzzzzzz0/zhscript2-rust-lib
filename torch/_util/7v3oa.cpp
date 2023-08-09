#include "ggv.cpp"

int main() {
	auto device = torch::Device("cuda");
	auto precision = torch::kFloat;
	auto downsampleRatio = 0.4;
	c10::optional<torch::Tensor> tensorRec0;
	c10::optional<torch::Tensor> tensorRec1;
	c10::optional<torch::Tensor> tensorRec2;
	c10::optional<torch::Tensor> tensorRec3;

	auto model = torch::jit::load("/zzzzzzzzzzz6/zzzzzzzzzzz4/opt/src/mk----/RobustVideoMatting/model/rvm_resnet50_fp32.torchscript");
	model.to(device);
	
	cv::Mat img = cv::imread("/_pic/美女/真人/1/11083470166a4830f2o.jpg");
	//wr__(img);

	auto tensorSrc = torch::from_blob(img.data, { img.rows, img.cols, 3 }, torch::kByte);
	tensorSrc = tensorSrc.to(device);
	tensorSrc = tensorSrc.permute({ 2,0,1 }).contiguous();
	tensorSrc = tensorSrc.to(precision).div(255);
	tensorSrc.unsqueeze_(0);

	std::vector<torch::IValue> v {};
	v.push_back(tensorSrc);
	v.push_back(tensorRec0);
	v.push_back(tensorRec1);
	v.push_back(tensorRec2);
	v.push_back(tensorRec3);
	v.push_back(downsampleRatio);
	auto outputs = model.forward(v).toList();

	const auto &fgr = outputs.get(0).toTensor()
		//.view({1, 4, 1, 1})
		;
	const auto &pha = outputs.get(1).toTensor()
		//.view({1, 4, 1, 1})
		;
	tensorRec0 = outputs.get(2).toTensor();
	tensorRec1 = outputs.get(3).toTensor();
	tensorRec2 = outputs.get(4).toTensor();
	tensorRec3 = outputs.get(5).toTensor();

	printf("z\n");
	auto tensorTargetBgr = torch::tensor({0.47,1.,0.6}).toType(precision).to(device).view({
		1, 3, 1, 1
		//1, 4, 1, 1
	});
	printf("z\n");
	auto res_tensor = (pha * fgr + (1 - pha) * tensorTargetBgr);

	auto wr2__ = [](auto res_tensor, auto fmt) {
		res_tensor = res_tensor.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
		wr__(res_tensor, fmt);
	};

	wr2__(res_tensor, CV_8UC3);
	wr2__(pha, CV_8U);

	wr2__(fgr, CV_8UC3);
	wr2__(tensorTargetBgr, CV_8UC3);

	return 0;
}
