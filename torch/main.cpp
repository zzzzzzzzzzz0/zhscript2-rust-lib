#include <torch/script.h>
#include <iostream>
#include <cstdarg>
#include "image_io.h"

class Devi_ {
public:
	torch::Device* h_;
};

class In_ {
public:
	torch::Tensor h_;
};

class Out_ {
public:
	at::Tensor h_;
};

class Model_ {
public:
	torch::jit::script::Module h_;
};

Model_* load__(const char* pt, Devi_* d, int i) {
	Model_* m = new Model_();
	try {
		switch(i) {
		case 0:
			m->h_ = torch::jit::load(pt, *d->h_);
			break;
		default:
			break;
		}
	} catch (const c10::Error& e) {
		delete m;
		std::cerr << e.msg() << std::endl;
		return nullptr;
	}
	return m;
}

extern "C" {

Devi_* devi__() {
	Devi_* d = new Devi_();
	torch::DeviceType dt = torch::kCPU;
	//dt = torch::kCUDA;
	d->h_ = new torch::Device(dt);
	return d;
}

In_* get_img__(const char* path, Devi_* d, size_t argc, ...) {
	In_* i = new In_();
	torch::IntArrayRef* shape = nullptr;
	try {
		torch::data::transforms::Normalize<> normalize_transform({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
		{
			int i2[2];
			switch(argc) {
				case 0: case 1: case 2: break;
				default: throw std::runtime_error("The number of parameters exceeded.");
			}
			va_list argv;
			va_start(argv, argc);
			for (size_t i = 0; i < argc; i++) {
				const char* s = va_arg(argv, const char*);
				i2[i] = std::stoi(s);
			}
			va_end(argv);
			switch(argc) {
				case 0:
					shape = new torch::IntArrayRef{};
					break;
				case 1:
					shape = new torch::IntArrayRef{i2[0]};
					break;
				case 2:
					shape = new torch::IntArrayRef{i2[1], i2[0]};
					break;
			}
		}
		i->h_ = image_io::load_image(path, *shape, normalize_transform).unsqueeze_(0).to(*d->h_);
	} catch (const std::exception& e) {
		delete i;
		std::cerr << e.what() << std::endl;
		i = nullptr;
	}
	if(shape)
		delete shape;
	return i;
}

Model_* load__(const char* pt, Devi_* d) {return load__(pt, d, 0);}

Out_* run__(Model_* m, In_* i) {
	std::vector<torch::jit::IValue> inputs;
	inputs.push_back(i->h_);

	Out_* o = new Out_();
	o->h_ = m->h_.forward(inputs).toTensor();
	return o;
}

void save_img__(Out_* o, const char* path) {
	torch::data::transforms::Normalize<> denormalize_transform({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
	auto image = denormalize_transform(o->h_[0].to(torch::kCPU).clone())
		.clamp_(0, 255);
	using image_io::ImageFormat;
	ImageFormat fmt = ImageFormat::JPG;
	if(path) {
		int i1 = -1, i2 = -1, i3 = -1;
		for(int i = 0;; i++) {
			char c = path[i];
			if(!c) {
				i3 = i;
				break;
			}
			switch(c) {
			case '/':
				i1 = i;
				break;
			case '.':
				i2 = i;
				break;
			}
		}
		if(i2 > i1 && i3 - i2 == 4) {
			char	c1 = path[i2 + 1],
					c2 = path[i2 + 2],
					c3 = path[i2 + 3];
			if(	(c1 == 'p' || c1 == 'P')
			&&	(c2 == 'n' || c2 == 'N')
			&&	(c3 == 'g' || c3 == 'G')) {
				fmt = ImageFormat::PNG;
			}
			else
			if(	(c1 == 'b' || c1 == 'B')
			&&	(c2 == 'm' || c2 == 'M')
			&&	(c3 == 'p' || c3 == 'P')) {
				fmt = ImageFormat::BMP;
			}
		}
	}
	image_io::save_image(image, path, 1, 0, /*false*/true, {}, false, 0, fmt);
}

void set_num_threads__(int i) {
	torch::set_num_threads(i);
}
int get_num_threads__() {
	return torch::get_num_threads();
}

}