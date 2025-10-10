#include "kllm.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    std::cout << "--- KLLM Interactive Test ---" << std::endl;
    std::cout << "Initializing KLLM subsystem..." << std::endl;

    // 1. Initialize the subsystem
    kllm_status_t status = kllm_initialize();
    if (status != KLLM_SUCCESS) {
        std::cerr << "Error: Failed to initialize KLLM subsystem." << std::endl;
        return 1;
    }

    // 2. Load a model
    std::cout << "Loading model... (This may take a moment)" << std::endl;
    kllm_model_t* model_handle = nullptr;
    // Note: This requires the model file to be present at this path.
    status = kllm_load_model("../models/tinyllama-2-1b-miniguanaco.Q4_K_M.gguf", &model_handle);
    if (status != KLLM_SUCCESS) {
        std::cerr << "Error: Failed to load model. Please ensure the model file exists." << std::endl;
        kllm_shutdown();
        return 1;
    }
    std::cout << "Model loaded. Type 'exit' to quit." << std::endl;
    std::cout << "------------------------------------" << std::endl;

    // 3. Interactive chat loop
    std::string input;
    while (true) {
        std::cout << "\n### Human: ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        std::vector<char> output_buffer(2048);
        status = kllm_run_inference(model_handle, input.c_str(), output_buffer.data(), output_buffer.size());

        if (status != KLLM_SUCCESS) {
            std::cerr << "Error: Failed to run inference." << std::endl;
        } else {
            std::cout << "### Assistant: " << output_buffer.data() << std::endl;
        }
    }

    // 4. Clean up
    std::cout << "\n------------------------------------" << std::endl;
    std::cout << "Unloading model and shutting down..." << std::endl;
    kllm_unload_model(model_handle);
    kllm_shutdown();

    std::cout << "--- Test Complete ---" << std::endl;
    return 0;
}