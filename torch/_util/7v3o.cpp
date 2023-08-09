#include "ggv.cpp"

int main() {
	auto device = torch::Device("cuda");
	//auto device = torch::Device("cpu");
	auto precision = torch::kFloat16;
	auto downsampleRatio = 0.4;
	c10::optional<torch::Tensor> tensorRec0;
	c10::optional<torch::Tensor> tensorRec1;
	c10::optional<torch::Tensor> tensorRec2;
	c10::optional<torch::Tensor> tensorRec3;

	auto model = torch::jit::load("/zzzzzzzzzzz6/zzzzzzzzzzz4/opt/src/mk----/RobustVideoMatting/model/rvm_mobilenetv3_fp16.torchscript");
	model.to(device);
	
	cv::Mat img = cv::imread("/_pic/美女/真人/1/11083470166a4830f2o.jpg");
	/*cv::Mat img;
	cv::resize(img2, img, cv::Size(384, 640));
	cv::cvtColor(img, img, cv::COLOR_BGR2RGB);*/
	//wr__(img);

	auto tensorSrc = torch::from_blob(img.data, { img.rows, img.cols, 3 }, torch::kByte);
	tensorSrc = tensorSrc.to(device);
	tensorSrc = tensorSrc.permute({ 2,0,1 }).contiguous();
	tensorSrc = tensorSrc.to(precision).div(255);
	tensorSrc.unsqueeze_(0);

	std::vector<torch::IValue> v { /*tensorSrc,tensorRec0,tensorRec1,tensorRec2,tensorRec3*/ };
	v.push_back(tensorSrc);
	v.push_back(tensorRec0);
	v.push_back(tensorRec1);
	v.push_back(tensorRec2);
	v.push_back(tensorRec3);
	v.push_back(downsampleRatio);
	auto outputs = model.forward(v).toList();
	
	auto wr2__ = [](auto res_tensor, auto fmt) {
		res_tensor = res_tensor.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
		wr__(res_tensor, fmt);
	};

	const auto &fgr = outputs.get(0).toTensor();
	const auto &pha = outputs.get(1).toTensor();
	tensorRec0 = outputs.get(2).toTensor();
	tensorRec1 = outputs.get(3).toTensor();
	tensorRec2 = outputs.get(4).toTensor();
	tensorRec3 = outputs.get(5).toTensor();

	auto tensorTargetBgr = torch::tensor({
		120.f / 255, 255.f / 255, 155.f / 255
		//0.,0.47,1.,0.6,
	}).toType(precision).to(device).view({
		1, 3, 1, 1
		//1, 4, 1, 1, 1
	});
	auto res_tensor = pha * fgr + (1 - pha) * tensorTargetBgr;

	wr2__(res_tensor, CV_8UC3);
	wr2__(pha, CV_8U);

	wr2__(fgr, CV_8UC3);
	wr2__(tensorTargetBgr, CV_8UC3);
	/*wr2__(tensorRec0);
	wr2__(tensorRec1);
	wr2__(tensorRec2);
	wr2__(tensorRec3);*/

	return 0;
}

/*
auto device = torch::Device("cuda");
    auto precision = torch::kFloat16;
    auto downsampleRatio = 0.4;
    c10::optional<torch::Tensor> tensorRec0;
    c10::optional<torch::Tensor> tensorRec1;
    c10::optional<torch::Tensor> tensorRec2;
    c10::optional<torch::Tensor> tensorRec3;

    auto model = torch::jit::load("rvm_mobilenetv3_fp16.torchscript");
    //! freeze error.
    //model  = torch::jit::freeze(model );
    model.to(device);

    //! imgSrc: RGB image data, such as QImage.
    auto tensorSrc = torch::from_blob(imgSrc.bits(), { imgSrc.height(),imgSrc.width(),3 }, torch::kByte);
    tensorSrc = tensorSrc.to(device);
    tensorSrc = tensorSrc.permute({ 2,0,1 }).contiguous();
    tensorSrc = tensorSrc.to(precision).div(255);
    tensorSrc.unsqueeze_(0);

    //! Inference
    auto outputs = model.forward({ tensorSrc,tensorRec0,tensorRec1,tensorRec2,tensorRec3,downsampleRatio }).toList();

    const auto &fgr = outputs.get(0).toTensor();
    const auto &pha = outputs.get(1).toTensor();
    tensorRec0 = outputs.get(2).toTensor();
    tensorRec1 = outputs.get(3).toTensor();
    tensorRec2 = outputs.get(4).toTensor();
    tensorRec3 = outputs.get(5).toTensor();

    //! Green target bgr
    auto tensorTargetBgr = torch::tensor({ 120.f / 255, 255.f / 255, 155.f / 255 }).toType(precision).to(device).view({ 1, 3, 1, 1 });
    //! Compound
    auto res_tensor = pha * fgr + (1 - pha) * tensorTargetBgr;

    res_tensor = res_tensor.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();


while (vCap.read(frame))
{
cv::cvtColor(frame, srcframe, cv::COLOR_BGR2RGB);

    auto src = torch::from_blob(srcframe.data, { srcframe.rows,srcframe.cols,3 }, torch::kByte);
    src = src.to(device);
    src = src.permute({ 2,0,1 }).contiguous();
    src = src.to(precision).div(255);
    src.unsqueeze_(0);


    //auto outputs = model.forward({ src, tRec0,tRec1,tRec2,tRec3,downsampleRatio }).toTuple()->elements();
    auto outputs = model.forward({ src, tRec0,tRec1,tRec2,tRec3,downsampleRatio }).toList();
    
    const auto& fgr = outputs.get(0).toTensor();
    const auto& pha = outputs.get(1).toTensor();
  
    tRec0 = outputs.get(2).toTensor();
    tRec1 = outputs.get(3).toTensor();
    tRec2 = outputs.get(4).toTensor();
    tRec3 = outputs.get(5).toTensor();

     auto com =  pha *fgr +  newbgr*(1 - pha);
   
    cv::Mat resultImg = torchTensortoCVMat(com);

    cv::cvtColor(resultImg, resultImg, COLOR_RGB2BGR);
    
    cv::imshow("demo", resultImg);
    if (waitKey(1) >= 0)
        break;
}
*/