#include <png++/png.hpp>

torch::Tensor ConvertRGBAintoTensor(png::image<png::rgba_pixel> &image){
    size_t width = image.get_width();
    size_t height = image.get_height();
    unsigned char *pointer = new unsigned char[width * height * 4];
    for (size_t j = 0; j < height; j++){
        for (size_t i = 0; i < width; i++){
            pointer[j * width * 4 + i * 4 + 0] = image[j][i].red;
            pointer[j * width * 4 + i * 4 + 1] = image[j][i].green;
            pointer[j * width * 4 + i * 4 + 2] = image[j][i].blue;
            pointer[j * width * 4 + i * 4 + 3] = image[j][i].alpha;
        }
    }
    torch::Tensor tensor = torch::from_blob(pointer, {image.get_height(), image.get_width(), 4}, torch::kUInt8).clone();  // copy
    tensor = tensor.permute({2, 0, 1});  // {H,W,C} ===> {C,H,W}
    delete[] pointer;
    return tensor;
}

png::image<png::rgba_pixel> ConvertTensorintoRGBA(torch::Tensor &tensor_){
    torch::Tensor tensor = tensor_.permute({1, 2, 0});  // {C,H,W} ===> {H,W,C}
    size_t width = tensor.size(1);
    size_t height = tensor.size(0);
    unsigned char *pointer = tensor.data_ptr<unsigned char>();
    png::image<png::rgba_pixel> image(width, height);
    for (size_t j = 0; j < height; j++){
        for (size_t i = 0; i < width; i++){
            image[j][i].red = pointer[j * width * 4 + i * 4 + 0];
            image[j][i].green = pointer[j * width * 4 + i * 4 + 1];
            image[j][i].blue = pointer[j * width * 4 + i * 4 + 2];
            image[j][i].alpha = pointer[j * width * 4 + i * 4 + 3];
        }
    }
    return image;
}
