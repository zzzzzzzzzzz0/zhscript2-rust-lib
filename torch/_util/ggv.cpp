#include <opencv2/opencv.hpp>
#include <torch/script.h>

int i_wr_ = 1;
void wr__(cv::Mat& comp) {
	std::string path;
	path += "/tmp/out";
	if(i_wr_ > 1)
		path += std::to_string(i_wr_);
	i_wr_++;
	path += ".png";
	printf("%s\n", path.c_str());
	cv::imwrite(path, comp);
}
void wr__(auto res_tensor, auto fmt) {
	cv::Mat comp(cv::Size(res_tensor.size(1), res_tensor.size(0)), fmt, res_tensor.data_ptr());
	wr__(comp);
}
