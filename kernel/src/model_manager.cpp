#include "kllm.h"
#include "pseudo_kernel.h"
#include <string>
#include <vector>

// Global instance of our pseudo-kernel.
// This is the single point of control for all KLLM core operations.
static PseudoKernel g_kernel;

// --- C-style API Wrappers ---

// Note: The kllm_model_t* handle is used to maintain API compatibility,
// but in this phase, it will always refer to the single global kernel instance.

kllm_status_t kllm_load_model(const char* model_path, kllm_model_t** model_handle) {
    if (!model_path || !model_handle) {
        return KLLM_ERROR_INVALID_ARG;
    }
    kllm_status_t status = g_kernel.load_model(model_path);
    if (status == KLLM_SUCCESS) {
        // Return a pointer to the kernel as the handle.
        *model_handle = reinterpret_cast<kllm_model_t*>(&g_kernel);
    }
    return status;
}

void kllm_unload_model(kllm_model_t* model_handle) {
    // The handle is ignored as we only have one global instance.
    g_kernel.unload_model();
}

kllm_status_t kllm_run_inference(kllm_model_t* model_handle, const char* input_text, char* output_buffer, size_t output_buffer_size) {
    if (!model_handle || !input_text || !output_buffer) {
        return KLLM_ERROR_INVALID_ARG;
    }

    std::string output_str;
    kllm_status_t status = g_kernel.run_inference(input_text, output_str);

    if (status == KLLM_SUCCESS) {
        strncpy(output_buffer, output_str.c_str(), output_buffer_size - 1);
        output_buffer[output_buffer_size - 1] = '\0';
    }
    return status;
}