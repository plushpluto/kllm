#include "kllm.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "--- KLLM API Functional Test ---" << std::endl;

    // 1. Initialize the subsystem
    kllm_status_t status = kllm_initialize();
    if (status != KLLM_SUCCESS) {
        std::cerr << "Error: Failed to initialize KLLM subsystem." << std::endl;
        return 1;
    }

    // 2. Load a model
    kllm_model_t* model_handle = nullptr;
    status = kllm_load_model("path/to/dummy_model.bin", &model_handle);
    if (status != KLLM_SUCCESS) {
        std::cerr << "Error: Failed to load model." << std::endl;
        kllm_shutdown();
        return 1;
    }

    // 3. Run inference
    const char* input = "Hello, world!";
    std::vector<char> output_buffer(128);
    status = kllm_run_inference(model_handle, input, output_buffer.data(), output_buffer.size());
    if (status != KLLM_SUCCESS) {
        std::cerr << "Error: Failed to run inference." << std::endl;
        kllm_unload_model(model_handle);
        kllm_shutdown();
        return 1;
    }

    std::cout << "Inference successful." << std::endl;
    std::cout << "Model response: \"" << output_buffer.data() << "\"" << std::endl;

    // 4. Clean up
    kllm_unload_model(model_handle);
    kllm_shutdown();

    std::cout << "--- Test Complete ---" << std::endl;
    return 0;
}