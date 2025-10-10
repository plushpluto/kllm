#ifndef PSEUDO_KERNEL_H
#define PSEUDO_KERNEL_H

#include "kllm.h"
#include "llama.h"
#include <string>
#include <vector>

// Represents the state of the AI model managed by the kernel.
enum class FSMState {
    IDLE,
    LOADING,
    LOADED,
    RUNNING,
    ERROR
};

// The core class responsible for managing the AI model and its lifecycle.
class PseudoKernel {
public:
    PseudoKernel();
    ~PseudoKernel();

    kllm_status_t initialize();
    void shutdown();

    kllm_status_t load_model(const std::string& model_path);
    void unload_model();

    kllm_status_t run_inference(const std::string& input_text, std::string& output_text);

private:
    llama_model* model_;
    llama_context* ctx_;
    FSMState state_;
};

#endif // PSEUDO_KERNEL_H