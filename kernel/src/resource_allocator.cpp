#include "kllm.h"
#include "llama.h"
#include <iostream>

// Implementation of the KLLM resource management functions.
// These functions will initialize and shut down the llama.cpp backend.

kllm_status_t kllm_initialize() {
    std::cout << "KLLM: Initializing llama.cpp backend." << std::endl;

    // Initialize llama.cpp backend
    llama_backend_init();

    std::cout << "KLLM: Llama.cpp backend initialized successfully." << std::endl;
    return KLLM_SUCCESS;
}

void kllm_shutdown() {
    std::cout << "KLLM: Shutting down llama.cpp backend." << std::endl;

    // Free the llama.cpp backend
    llama_backend_free();

    std::cout << "KLLM: Llama.cpp backend shut down." << std::endl;
}