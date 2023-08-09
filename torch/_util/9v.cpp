#include "../main2.cpp"

int main(int argc, char** argv) {
	if(argc >= 4 && argv[1][0] == 'r') {
		Devi_* d = devi__();

		Model_* m = load__(argv[2], d);
		if(!m)
			return 2;

		In_* img = get_img__(argv[3], d, 0);
		if(!img)
			return 3;

		Out_* o = run__(m, img, 1);
		if(!o)
			return 4;

		std::vector<std::vector<std::string>*>* p = new std::vector<std::vector<std::string>*>();
		non_max_suppression__(o, p);
		{
			int i1 = 0;
			for(; i1 < (int)p->size() && i1 >= 0;) {
				std::vector<std::string>* v = (*p)[i1];
				for(size_t i = 0; i < v->size(); i++)
					std::cout << (*v)[i] << " ";
				std::cout << std::endl;
				i1++;
			}
		}
		return 0;
	}
	if(argc >= 12 && argv[1][0] == 'p') {
		std::vector<std::string> args;
		for (size_t i = 6; i < argc; i++)
			args.push_back(argv[i]);
		int err = save_img__(argv[2], argv[3], std::atof(argv[4]), std::atof(argv[5]), args);
		return 0;
	}
	std::cerr
		<< "r mod img" << " [" << img_w_ << "]" << " [" << img_h_ << "]" << std::endl
		<< "p img img2 w-s h-s left top right bottom score classID ..." << std::endl
		;
	return 200;
}
