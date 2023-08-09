#include <iostream>
#include <torch/torch.h>
#include "ggp.cpp"

int main(void){

    // Input PNG-image
    png::image<png::rgba_pixel> imageI("/_pic/美女/真人/1/49.png");

    // Convert png::image into torch::Tensor
    torch::Tensor tensor = ConvertRGBAintoTensor(imageI);
    std::cout << "C:" << tensor.size(0) << " H:" << tensor.size(1) << " W:" << tensor.size(2) << std::endl;

    // Convert torch::Tensor into png::image
    png::image<png::rgba_pixel> imageO = ConvertTensorintoRGBA(tensor);

    // Output PNG-image
    imageO.write("/tmp/out.png");

    return 0;
}
