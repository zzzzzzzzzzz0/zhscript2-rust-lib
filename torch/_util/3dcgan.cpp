#include <torch/script.h>
#include <torch/torch.h>

#include <iostream>
#include <memory>

int main(int argc, const char* argv[]) {
	std::cout <<"cuda::is_available" << torch::cuda::is_available() << std::endl;

  torch::jit::script::Module module;
  try {
    module = torch::jit::load("saved_models/candy.pt");
  }
  catch (const c10::Error& e) {
    std::cerr << e.msg() << "\n";
    return -1;
  }

}
