#include "ggv.cpp"
#include "ggp.cpp"

void z__(const torch::Tensor &res_tensor, const char* s2 = "", int i2 = 0) {
	std::cout << s2 << "," << i2 << std::endl;
	size_t k = 3;
	auto tensor = res_tensor.mul(255);
	switch(i2) {
	case 0:
		tensor = tensor.permute({ 0,2,3,1 })[0];
		break;
	case 1:
		k = 1;
		tensor = tensor.permute({ 0,2,3,1 })[0];
		break;
	}
	tensor = tensor.to(torch::kU8).contiguous().cpu()
	;
	size_t width = tensor.size(1);
	size_t height = tensor.size(0);
	unsigned char *pointer = tensor.data_ptr<unsigned char>();
	png::image<png::rgba_pixel> image(width, height);
	for (size_t j = 0; j < height; j++){
		for (size_t i = 0; i < width; i++){
			unsigned char a = pointer[j * width * k + i * k + 0];
			unsigned char b = pointer[j * width * k + i * k + 1];
			unsigned char c = pointer[j * width * k + i * k + 2];
			//printf("%d %d %d\n",a,b,c);
			if(
			a == 119 && b == 255 && c == 153
			) {
				image[j][i].red   =
				image[j][i].green =
				image[j][i].blue  =
				image[j][i].alpha = 0;
			} else {
				image[j][i].red   = c;
				image[j][i].green = b;
				image[j][i].blue  = a;
				image[j][i].alpha = 255;
			}
		}
	}
	image.write("/tmp/out" + std::string(s2) + ".png");
}
void zz__(const torch::Tensor &res_tensor, const torch::Tensor &pha0, const char* s2 = "") {
	size_t k = 3;
	auto tensor = res_tensor.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
	auto pha = pha0.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
	size_t width = tensor.size(1);
	size_t height = tensor.size(0);
	unsigned char *pointer = tensor.data_ptr<unsigned char>();
	unsigned char *p = pha.data_ptr<unsigned char>();
	png::image<png::rgba_pixel> image(width, height);
	unsigned char min = (unsigned char)(255 * 0.75);
	std::cout << width << "x" << height << " " << (int)min << std::endl;
	size_t min_j = 0, max_j = 0, min_i = height, max_i = 0;
	bool can_min_j = true;
	std::vector<std::string> v;
	for (size_t j = 0; j < height; j++){
		for (size_t i = 0; i < width; i++){
			unsigned char a = p[j * width + i + 0];
			unsigned char b = p[j * width + i + 1];
			unsigned char c = p[j * width + i + 2];
			std::string s = 
				std::to_string((int)a) +
				std::string(",") +
				std::to_string((int)b) +
				std::string(",") +
				std::to_string((int)c);
			if(std::find(v.begin(), v.end(), s) == v.end()) {
				//std::cout << s << std::endl;
				v.push_back(s);
			}
			if(a <= min && b <= min && c <= min) {
				image[j][i].red   =
				image[j][i].green =
				image[j][i].blue  =
				image[j][i].alpha = 0;

				if(can_min_j && min_j < j) min_j = j;
			} else {
				a = pointer[j * width * k + i * k + 0];
				b = pointer[j * width * k + i * k + 1];
				c = pointer[j * width * k + i * k + 2];
				image[j][i].red   = c;
				image[j][i].green = b;
				image[j][i].blue  = a;
				image[j][i].alpha = 255;

				can_min_j = false;
				if(min_i > i) min_i = i;
				max_j = j;
				if(max_i < i) max_i = i;
			}
		}
	}
	size_t w = max_i-min_i, h = max_j-min_j;
	std::cout << "遮罩多少种？" << v.size() << "  积极剪裁 " << min_i << "," << min_j << "," << (w) << "," << (h) << std::endl;
	{
		png::image<png::rgba_pixel> img(w + 1, h + 1);
		for (size_t j = min_j; j <= max_j; j++){
			for (size_t i = min_i; i <= max_i; i++){
				size_t j2 = j - min_j, i2 = i - min_i;
				img[j2][i2].red   = image[j][i].red;
				img[j2][i2].green = image[j][i].green;
				img[j2][i2].blue  = image[j][i].blue;
				img[j2][i2].alpha = image[j][i].alpha;
			}
		}
		img.write("/tmp/out" + std::string(s2) + "-ac.png");
	}
	image.write("/tmp/out" + std::string(s2) + ".png");
}

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

	cv::Mat img = cv::imread(
		"/_pic/美女/真人/1/11083470166a4830f2o.jpg"
	);

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

	const auto &fgr = outputs.get(0).toTensor();
	const auto &pha = outputs.get(1).toTensor();
	tensorRec0 = outputs.get(2).toTensor();
	tensorRec1 = outputs.get(3).toTensor();
	tensorRec2 = outputs.get(4).toTensor();
	tensorRec3 = outputs.get(5).toTensor();
	
	//std::cout << pha << std::endl;

	auto tensorTargetBgr = torch::tensor({0.47,1.,0.6}).toType(precision).to(device).view({1, 3, 1, 1});
	auto res_tensor = pha * fgr + (1 - pha) * tensorTargetBgr;
	z__(res_tensor, "4");
	
	auto t = pha * fgr + (1 - pha);
	z__(t, "5");
	
	z__(fgr, "2");
	z__(pha, "3", 1);
	zz__(fgr, pha);

	return 0;
}
