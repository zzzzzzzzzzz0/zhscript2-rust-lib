#include <torch/torch.h>
#include "image_io.h"
#include "stb_image_write.h"

class In_ {
public:
	torch::Tensor h_;
	c10::optional<torch::Tensor> h3_;
	int e_ = -1;
};

#include "main2-1.cpp"

extern "C" {

void get_img__(rust_add___ add, void* env, const char* path, Devi_* d, size_t argc, ...) {
	In_* ret = new In_();
	try {
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
				/*case 2:
					break;*/
				case 3:
					retyp = std::stoi(s);
					break;
				}
			}
			va_end(argv);
		}
		for(;;) {
			torch::data::transforms::Normalize<> normalize_transform({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
			int imgw = 0, imgh = 0;
			torch::Tensor imgTensor = image_io::load_image(path, {}, normalize_transform, &imgw, &imgh);
			if(imgw == 0 || imgh == 0) {
				break;
			}
			switch(retyp) {
			case 16: case 32: {
				imgTensor = imgTensor.contiguous();
				imgTensor = imgTensor.to(retyp == 16 ? torch::kFloat16 : torch::kFloat32);
				break; }
			default: {
				imgTensor = imgTensor.toType(torch::kFloat);
				break; }
			}
			imgTensor = imgTensor.to(*d->h_);
			imgTensor = imgTensor.unsqueeze(0);
			switch(retyp) {
			case 1:
				imgTensor = imgTensor.permute({ 0, 3, 1, 2 });
				imgTensor[0][0] = imgTensor[0][0].sub(114).div(255.0);
				imgTensor[0][1] = imgTensor[0][1].sub(121).div(255.0);
				imgTensor[0][2] = imgTensor[0][2].sub(134).div(255.0);
				break;
			}

			ret->h_ = imgTensor;

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
	return 10;
}

int save_img2__(Out_* o, In_* i, int imgw2, int imgh2, int imgw, int imgh, const char* path2) {
	return 0;
}

int save_img3__(Out_* o1, Out_* o2, const char* boshi, bool autocut, Devi_* d, const char* path2) {
	try {
		const auto &pha0 = o2->h_;
		const auto &fgr0 = o1->h_;

		auto fgr = fgr0.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
		auto pha = pha0.mul(255).permute({ 0,2,3,1 })[0].to(torch::kU8).contiguous().cpu();
		size_t width = fgr.size(1);
		size_t height = fgr.size(0);
		unsigned char *pf = fgr.data_ptr<unsigned char>();
		unsigned char *p  = pha.data_ptr<unsigned char>();
		unsigned char* p2;
		unsigned char min = (unsigned char)(255 * std::stof(boshi));
		size_t min_j = 0, max_j = 0, min_i = width, max_i = 0;
		if(autocut) {
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
		} else {
			max_j = height - 1;
			min_i = 0;
			max_i = width - 1;
		}

		size_t w = max_i-min_i;
		size_t h = max_j-min_j;
		size_t len = (w + 1) * (h + 1) * 4;
		p2 = new unsigned char[len];
		if(!p2) {
			return 0;
		}
		for (size_t j = min_j; j <= max_j; j++){
			for (size_t i = min_i; i <= max_i; i++){
				unsigned char a = p[j * width + i + 0];
				unsigned char b = p[j * width + i + 1];
				unsigned char c = p[j * width + i + 2];
				size_t o = ((j - min_j) * w + (i - min_i)) * 4;
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

		stbi_write_png(path2, w, h, 4, p2, 0);

		delete p2;
		return 1;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}

}
