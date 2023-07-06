#include <iostream>
#include <cstdio>
#include <torch/torch.h>
#include <opencv2/opencv.hpp>
#include <png++/png.hpp>

class In_ {
public:
	torch::Tensor h_;
	cv::Mat h2_;
	c10::optional<torch::Tensor> h3_;
	int e_ = -1;
};

#include "main2-1.cpp"

static int save_img__(const char* path, const char* path2, const char* cf2, const std::vector<std::string>& args) {
	if(args.size() % 7 != 0)
		return 3;
	//std::cout << path << std::endl;
	cv::Mat frame = cv::imread(path);
	if (frame.empty()) {
		return 1;
	}
	float w_s = frame.cols / (float)img_w_, h_s = frame.rows / (float)img_h_;
	int thickness = 2, thickness2 = 1;
	float fontScale = 1, yx = 2;
	std::vector<cv::Scalar> color, color2;
	{
		const char* cf = "8b9a61/ffffff,e90cd8/ffffff,2,1,1,2";
		if(cf2 && cf2[0]) cf = cf2;
		int oft = 0;
		for(;;) {
			int r, g, b, r2, g2, b2, n = -1;
			int i = sscanf(cf + oft, "%02x%02x%02x/%02x%02x%02x%n,", &r, &g, &b, &r2, &g2, &b2, &n);
			if(i != 6)
				break;
			if(n == -1)
				throw std::runtime_error("c++1y?");
			color.push_back(cv::Scalar(b, g, r));
			color2.push_back(cv::Scalar(b2, g2, r2));
			oft += n + 1;
		}
		sscanf(cf + oft, "%d,%d,%f,%f", &thickness, &thickness2, &fontScale, &yx);
	}
	int fontFace = cv::FONT_HERSHEY_PLAIN;
	for (size_t i = 0; i < args.size();) {
		float left   = std::stof(args[i++]) * w_s;
		float top    = std::stof(args[i++]) * h_s;
		float right  = std::stof(args[i++]) * w_s;
		float bottom = std::stof(args[i++]) * h_s;
		float score  = std::stof(args[i++]);
		int colori   = std::stoi(args[i++]);
		auto classname = args[i++];
		colori %= color.size();
		//std::cout << left << "," << top << "," << right << "," << bottom << " " << score << " " << classname << std::endl;

		cv::rectangle(frame, cv::Rect(left, top, (right - left), (bottom - top)), color[colori], thickness);

		auto s = classname + ": " + cv::format("%.2f", score);
		//std::cout << s << std::endl;
		cv::Size si = cv::getTextSize(s, fontFace, fontScale, thickness2, nullptr);
		si.height += yx * 2;
		si.width += 0;
		left -= thickness / 2;
		float top2 = top - si.height, top3 = 0;
		if(top2 < top3) top2 = top3;
		cv::rectangle(frame, cv::Rect(left, top2, si.width, si.height), color[colori], /*CV_FILLED*/-1);
		top2 = top - yx; top3 = si.height - yx;
		if(top2 < top3) top2 = top3;
		cv::putText(frame, s, cv::Point(left, top2), fontFace, fontScale, color2[colori], thickness2);
	}
	//std::cout << path2 << std::endl;
	if(!cv::imwrite(path2, frame))
		return 2;
	return 0;
}

extern "C" {

void get_img__(rust_add___ add, void* env, const char* path, Devi_* d, size_t argc, ...) {
	In_* ret = new In_();
	try {
		int interpolation = cv::INTER_LINEAR;
		int retyp = 0;
		{
			va_list argv;
			va_start(argv, argc);
			for (size_t i = 0; i < argc; i++) {
				const char* s = va_arg(argv, const char*);
				if(!s || !s[0]) continue;
				switch(i) {
				case 0:
					img_w_ = std::stoi(s);
					break;
				case 1:
					img_h_ = std::stoi(s);
					break;
				case 2:
					switch(s[0]) {
					case 'n': interpolation = cv::INTER_NEAREST; break;
					case 'a': interpolation = cv::INTER_AREA; break;
					case 'c': interpolation = cv::INTER_CUBIC; break;
					case 'l': interpolation = cv::INTER_LANCZOS4; break;
					default:
						interpolation = std::stoi(s);
						break;
					}
					break;
				case 3:
					retyp = std::stoi(s);
					break;
				}
			}
			va_end(argv);
		}
		for(;;) {
			cv::Mat frame = cv::imread(path);
			int imgw = frame.cols, imgh = frame.rows;
			//std::cout << path << " " << imgw << "x" << imgh << " " << retyp << std::endl;
			if(imgw == 0 || imgh == 0) {
				break;
			}
			torch::Tensor imgTensor;
			switch(retyp) {
			case 16: case 32: {
				imgTensor = torch::from_blob(frame.data, {frame.rows, frame.cols, 3}, torch::kByte);
				imgTensor = imgTensor.contiguous();
				imgTensor = imgTensor.to(retyp == 16 ? torch::kFloat16 : torch::kFloat32);
				break; }
			default: {
				cv::Mat img;
				cv::resize(frame, img, cv::Size(img_w_, img_h_), interpolation);
				switch(retyp) {
				case 0:
					cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
					break;
				}
				imgTensor = torch::from_blob(img.data, {img.rows, img.cols, 3}, torch::kByte);
				imgTensor = imgTensor.toType(torch::kFloat);
				break; }
			}
			imgTensor = imgTensor.to(*d->h_);
			switch(retyp) {
			default:
				imgTensor = imgTensor.permute({2,0,1});
				imgTensor = imgTensor.div(255);
				imgTensor = imgTensor.unsqueeze(0);
				break;
			case 1:
				imgTensor = imgTensor.unsqueeze(0);
				imgTensor = imgTensor.permute({ 0, 3, 1, 2 });
				imgTensor[0][0] = imgTensor[0][0].sub(114).div(255.0);
				imgTensor[0][1] = imgTensor[0][1].sub(121).div(255.0);
				imgTensor[0][2] = imgTensor[0][2].sub(134).div(255.0);
				break;
			}

			ret->h_ = imgTensor;
			ret->h2_ = frame;

			add__(add, ret, false, env);
			add__(add, imgw, true, env);
			add__(add, imgh, true, env);
			return;
		}
	} catch (const c10::Error& e) {
		std::cerr << e.msg() << std::endl;
	} catch(std::invalid_argument &ia) {
		std::cerr << ia.what() <<std::endl;
	}
	delete ret;
	add("0", false, env);
}

#include "main2-2.cpp"

int save_img__(const char* path, const char* path2, size_t argc, ...) {
	//std::cout << w_s << "," << h_s << "," << argc << std::endl;
	const char* cf = "";
	try {
		std::vector<std::string> args;
		va_list argv;
		va_start(argv, argc);
		for (size_t i = 0; i < argc; i++) {
			const char* s = va_arg(argv, const char*);
			switch(i) {
			case 0:
				cf = s;
				continue;
			}
			args.push_back(s);
		}
		va_end(argv);
		return save_img__(path, path2, cf, args);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 10;
}

int save_img2__(Out_* o, In_* i, int imgw2, int imgh2, int imgw, int imgh, const char* path2) {
	std::cout << "save_img2__" << o << "," << imgw2 << "," << imgh2 << "," << imgw << "," << imgh << "," << path2 << std::endl;
	std::string path3 = path2;
	path3 = path3.substr(0, path3.rfind("."));

	torch::Tensor out_tensor = o->h_;
	cv::Mat orgimg = i->h2_.clone();
	try {
		out_tensor = out_tensor.squeeze().detach();
		out_tensor = out_tensor.mul(255).add_(0.5).clamp_(0, 255).to(torch::kU8);
		out_tensor = out_tensor.to(torch::kCPU);
		std::cout << " " << out_tensor.numel() << std::endl;

		cv::Mat alphaImg(imgh2, imgw2, CV_8UC1);
		//std::memcpy((void*)alphaImg.data, out_tensor.data_ptr(), sizeof(torch::kU8) * out_tensor.numel());
		cv::imwrite((path3 + "2.png").c_str(), alphaImg);

		resize(alphaImg, alphaImg, cv::Size(imgw, imgh), 0, 0, cv::INTER_CUBIC);

		cv::Mat bg(orgimg.size(), orgimg.type(), cv::Scalar(255, 255, 255));
		cv::imwrite((path3 + "3.png").c_str(), bg);
		cv::Mat alpha, comp;
		cvtColor(alphaImg, alpha, cv::COLOR_GRAY2BGR);
		comp = orgimg.clone();
		cv::imwrite((path3 + "4.png").c_str(), comp);

		for (int i = 0; i < alpha.rows; i++)
			for (int j = 0; j < alpha.cols; j++)
			{
				cv::Vec3b alpha_p = alpha.at<cv::Vec3b>(i, j);
				cv::Vec3b bg_p = bg.at<cv::Vec3b>(i, j);
				cv::Vec3b img_p = orgimg.at<cv::Vec3b>(i, j);
				if (alpha_p[0] > 210)
				{
					alpha_p[0] = 255;
					alpha_p[1] = 255;
					alpha_p[2] = 255;
				}
				else
				{
					alpha_p[0] = 0;
					alpha_p[1] = 0;
					alpha_p[2] = 0;
				}
				comp.at<cv::Vec3b>(i, j)[0] = int(img_p[0] * (alpha_p[0] / 255.0) + bg_p[0] * (1.0 - alpha_p[0] / 255.0));
				comp.at<cv::Vec3b>(i, j)[1] = int(img_p[1] * (alpha_p[1] / 255.0) + bg_p[1] * (1.0 - alpha_p[1] / 255.0));
				comp.at<cv::Vec3b>(i, j)[2] = int(img_p[2] * (alpha_p[2] / 255.0) + bg_p[2] * (1.0 - alpha_p[2] / 255.0));
			}

		cv::imwrite(path2, comp);

		return 1;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}

int save_img3__(Out_* o1, Out_* o2, const char* boshi, bool autocut, Devi_* d, const char* path2) {
	try {
		const auto &pha0 = o2->h_;
		const auto &fgr0 = o1->h_;

		auto tensor = fgr0.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
		auto pha = pha0.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
		size_t width = tensor.size(1);
		size_t height = tensor.size(0);
		unsigned char *pointer = tensor.data_ptr<unsigned char>();
		unsigned char *p = pha.data_ptr<unsigned char>();
		png::image<png::rgba_pixel> image(width, height);
		unsigned char min = (unsigned char)(255 * std::stof(boshi));

		size_t min_j = 0, max_j = 0, min_i = width, max_i = 0;
		bool can_min_j = true;

		for (size_t j = 0; j < height; j++){
			for (size_t i = 0; i < width; i++){
				unsigned char a = p[j * width + i + 0];
				unsigned char b = p[j * width + i + 1];
				unsigned char c = p[j * width + i + 2];
				if(a <= min && b <= min && c <= min) {
					image[j][i].red   =
					image[j][i].green =
					image[j][i].blue  =
					image[j][i].alpha = 0;

					if(can_min_j && min_j < j) min_j = j;
				} else {
					a = pointer[j * width * 3 + i * 3 + 0];
					b = pointer[j * width * 3 + i * 3 + 1];
					c = pointer[j * width * 3 + i * 3 + 2];
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
		if(autocut) {
			size_t w = max_i-min_i, h = max_j-min_j;
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
			img.write(path2);
		} else
			image.write(path2);
		return 1;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}

}
