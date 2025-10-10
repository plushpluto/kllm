#include "kllm.h"
#include "llama.h"
#include "common.h"
#include <iostream>
#include <vector>
#include <cstring>

// The internal structure for a KLLM model instance.
// The kllm_model_t in the public API is an opaque pointer to this.
struct kllm_model {
    llama_model* model = nullptr;
    llama_context* ctx = nullptr;
};

// --- Model Loading and Unloading ---

kllm_status_t kllm_load_model(const char* model_path, kllm_model_t** model_handle) {
    if (!model_path || !model_handle) {
        return KLLM_ERROR_INVALID_ARG;
    }

    std::cout << "KLLM: Loading model from " << model_path << std::endl;

    auto* new_kllm_model = new kllm_model();

    llama_model_params model_params = llama_model_default_params();
    new_kllm_model->model = llama_load_model_from_file(model_path, model_params);
    if (new_kllm_model->model == nullptr) {
        std::cerr << "Error: Failed to load model from " << model_path << std::endl;
        delete new_kllm_model;
        return KLLM_ERROR_MODEL_LOAD_FAILED;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048; // Context size
    ctx_params.n_threads = 4;
    ctx_params.n_threads_batch = 4;

    new_kllm_model->ctx = llama_new_context_with_model(new_kllm_model->model, ctx_params);
    if (new_kllm_model->ctx == nullptr) {
        std::cerr << "Error: Failed to create llama_context." << std::endl;
        llama_free_model(new_kllm_model->model);
        delete new_kllm_model;
        return KLLM_ERROR_MODEL_LOAD_FAILED;
    }

    *model_handle = new_kllm_model;
    std::cout << "KLLM: Model loaded successfully." << std::endl;
    return KLLM_SUCCESS;
}

void kllm_unload_model(kllm_model_t* model_handle) {
    if (!model_handle) {
        return;
    }
    std::cout << "KLLM: Unloading model." << std::endl;
    llama_free(model_handle->ctx);
    llama_free_model(model_handle->model);
    delete model_handle;
}

// --- Inference ---

kllm_status_t kllm_run_inference(kllm_model_t* model_handle, const char* input_text, char* output_buffer, size_t output_buffer_size) {
    if (!model_handle || !input_text || !output_buffer) {
        return KLLM_ERROR_INVALID_ARG;
    }

    std::string prompt = "### Human: " + std::string(input_text) + "\n### Assistant:";

    // Tokenize the prompt
    std::vector<llama_token> tokens_list = llama_tokenize(model_handle->model, prompt, true);
    int n_tokens = tokens_list.size();

    // Check if prompt is too long
    if (n_tokens > llama_n_ctx(model_handle->ctx) - 4) {
        std::cerr << "Error: Prompt is too long." << std::endl;
        return KLLM_ERROR_INVALID_ARG;
    }

    // Clear the KV cache
    llama_kv_cache_clear(model_handle->ctx);

    // Evaluate the initial prompt
    if (llama_decode(model_handle->ctx, llama_batch_get_one(tokens_list.data(), n_tokens, 0, 0))) {
        std::cerr << "Error: Initial llama_decode failed." << std::endl;
        return KLLM_ERROR_INFERENCE_FAILED;
    }

    // Main prediction loop
    std::string result_text = "";
    const int max_new_tokens = 256;
    int n_cur = n_tokens;

    while(n_cur <= llama_n_ctx(model_handle->ctx) && n_cur < n_tokens + max_new_tokens) {
        // Get logits for the last token
        auto* logits = llama_get_logits_ith(model_handle->ctx, n_cur - 1);

        // Sample the next token
        llama_token new_token_id = llama_sample_token_greedy(model_handle->ctx, nullptr, logits, 1, n_cur - 1);

        // Check for end-of-sequence token
        if (new_token_id == llama_token_eos(model_handle->model)) {
            break;
        }

        // Convert token to piece and append to result
        result_text += llama_token_to_piece(model_handle->ctx, new_token_id);

        // Prepare for the next iteration
        llama_batch batch = { 1, &new_token_id, nullptr, n_cur };
        if (llama_decode(model_handle->ctx, batch)) {
            std::cerr << "Error: Subsequent llama_decode failed." << std::endl;
            return KLLM_ERROR_INFERENCE_FAILED;
        }
        n_cur++;
    }

    // Copy the result to the output buffer
    strncpy(output_buffer, result_text.c_str(), output_buffer_size - 1);
    output_buffer[output_buffer_size - 1] = '\0';

    return KLLM_SUCCESS;
}