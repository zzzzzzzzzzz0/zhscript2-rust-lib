
Devi_* devi__(char* s) {
	Devi_* d = new Devi_();
	try {
		if(s == std::string("auto")) {
			d->h_ = new torch::Device(torch::cuda::is_available() ? torch::kCUDA : torch::kCPU);
		} else {
			d->h_ = new torch::Device(s);
		}
	} catch (const c10::Error& e) {
		std::cerr << e.msg() << std::endl;
		delete d;
		d = nullptr;
	}
	return d;
}

In_* h3__() {
	In_* ret = new In_();
	ret->e_ = 3;
	return ret;
}


Model_* load__(const char* pt, Devi_* d) {
	Model_* ret = new Model_();
	try {
		torch::jit::script::Module m0 = torch::jit::load(pt, *d->h_);
		ret->h_ = m0;
	} catch (const c10::Error& e) {
		std::cerr << e.msg() << std::endl;
		delete ret;
		ret = nullptr;
	}
	return ret;
}

void run__(rust_add___ add, void* env, Model_* m, int of, size_t argc, ...) {
	try {
		std::vector<torch::IValue> v {};
		{
			va_list argv;
			va_start(argv, argc);
			for (size_t i = 0; i < argc; i++) {
				const char* s = va_arg(argv, const char*);
				//printf("%s\n", s);
				if(s[1] == ':') {
					switch(s[0]) {
					case 'f':
						v.push_back(std::stod(s + 2));
						continue;
					}
				}
				In_* in = (In_*)std::stol(s);
				//printf("%d\n", in->e_);
				switch(in->e_) {
				case 3:
					v.push_back(in->h3_);
					break;
				default:
					v.push_back(in->h_);
					break;
				}
			}
		}
		auto f = m->h_.forward(v);
		switch(of) {
		case 0: {
			Out_* ret = new Out_();
			ret->h_ = f.toTensor();
			add__(add, ret, false, env);
			break; }
		case 1: {
			Out_* ret = new Out_();
			ret->h_ = f.toTuple()->elements()[0].toTensor();
			add__(add, ret, false, env);
			break; }
		case 2: {
			auto ls = f.toList();
			for(size_t i = 0; i < ls.size(); i++) {
				Out_* ret = new Out_();
				ret->h_ = ls.get(i).toTensor();
				add__(add, ret, i != 0, env);
			}
			break; }
		default:
			throw c10::Error(std::to_string(of), "of", nullptr);
		}
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

void non_max_suppression__(Out_* o, std::vector<std::vector<std::string>*>* ret) {
	std::vector<torch::Tensor> dets = non_max_suppression(o->h_, 0.4, 0.5);
	for (size_t i0=0; i0 < dets.size(); ++ i0) {
		for (int i=0; i < dets[i0].sizes()[0]; ++ i) {
			std::vector<std::string>* v = new std::vector<std::string>();
			for (int i2=0; i2 < dets[0][i].sizes()[0]; ++ i2) {
				std::string s = std::to_string(dets[0][i][i2].item().toFloat());
				for(;;) {
					size_t len = s.length();
					auto rm0 = [&]() {
						s = s.substr(0, len - 1);
					};
					if(len <= 1)
						break;
					char c = s[len - 1];
					if(c == '0') {
						rm0();
						continue;
					}
					if(c == '.')
						rm0();
					break;
				}
				v->push_back(s);
			}
			ret->push_back(v);
		}
	}
}

void set_num_threads__(int i) {
	torch::set_num_threads(i);
}
int get_num_threads__() {
	return torch::get_num_threads();
}

float test_f__(float f1, float f2) {
	float f3 = f1 + f2 * 100 + 10000;
	std::cout << f1 << "," << f2 << " " << f3 << std::endl;
	return f3;
}
