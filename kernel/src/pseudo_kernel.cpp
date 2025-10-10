#include "pseudo_kernel.h"
#include "common.h"
#include <iostream>
#include <vector>
#include <cstring>

// --- Constructor and Destructor ---

PseudoKernel::PseudoKernel() : model_(nullptr), ctx_(nullptr), state_(FSMState::IDLE) {
    std::cout << "PseudoKernel: Instance created. Initial state: IDLE" << std::endl;
}

PseudoKernel::~PseudoKernel() {
    if (state_ != FSMState::IDLE) {
        shutdown();
    }
    std::cout << "PseudoKernel: Instance destroyed." << std::endl;
}

// --- Lifecycle Management ---

kllm_status_t PseudoKernel::initialize() {
    if (state_ != FSMState::IDLE) {
        std::cerr << "Kernel already initialized or in an error state." << std::endl;
        return KLLM_ERROR_GENERAL;
    }
    std::cout << "PseudoKernel: Initializing llama.cpp backend." << std::endl;
    llama_backend_init();
    std::cout << "PseudoKernel: Backend initialized successfully." << std::endl;
    return KLLM_SUCCESS;
}

void PseudoKernel::shutdown() {
    if (state_ == FSMState::LOADED || state_ == FSMState::RUNNING) {
        unload_model();
    }
    std::cout << "PseudoKernel: Shutting down llama.cpp backend." << std::endl;
    llama_backend_free();
    state_ = FSMState::IDLE;
    std::cout << "PseudoKernel: State -> IDLE" << std::endl;
}

// --- Model Management ---

kllm_status_t PseudoKernel::load_model(const std::string& model_path) {
    state_ = FSMState::LOADING;
    std::cout << "PseudoKernel: State -> LOADING" << std::endl;

    llama_model_params model_params = llama_model_default_params();
    model_ = llama_load_model_from_file(model_path.c_str(), model_params);
    if (model_ == nullptr) {
        std::cerr << "Error: Failed to load model from " << model_path << std::endl;
        state_ = FSMState::ERROR;
        std::cout << "PseudoKernel: State -> ERROR" << std::endl;
        return KLLM_ERROR_MODEL_LOAD_FAILED;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048;
    ctx_params.n_threads = 4;
    ctx_params.n_threads_batch = 4;

    ctx_ = llama_new_context_with_model(model_, ctx_params);
    if (ctx_ == nullptr) {
        std::cerr << "Error: Failed to create llama_context." << std::endl;
        llama_free_model(model_);
        model_ = nullptr;
        state_ = FSMState::ERROR;
        std::cout << "PseudoKernel: State -> ERROR" << std::endl;
        return KLLM_ERROR_MODEL_LOAD_FAILED;
    }

    state_ = FSMState::LOADED;
    std::cout << "PseudoKernel: State -> LOADED" << std::endl;
    return KLLM_SUCCESS;
}

void PseudoKernel::unload_model() {
    if (ctx_) {
        llama_free(ctx_);
        ctx_ = nullptr;
    }
    if (model_) {
        llama_free_model(model_);
        model_ = nullptr;
    }
    state_ = FSMState::IDLE;
    std::cout << "PseudoKernel: State -> IDLE" << std::endl;
}

// --- Inference ---

kllm_status_t PseudoKernel::run_inference(const std::string& input_text, std::string& output_text) {
    if (state_ != FSMState::LOADED) {
        std::cerr << "Error: Model not loaded. Current state: " << static_cast<int>(state_) << std::endl;
        return KLLM_ERROR_GENERAL;
    }

    state_ = FSMState::RUNNING;
    std::cout << "PseudoKernel: State -> RUNNING" << std::endl;

    std::string prompt = "### Human: " + input_text + "\n### Assistant:";
    std::vector<llama_token> tokens_list = llama_tokenize(model_, prompt, true);

    if (tokens_list.size() > llama_n_ctx(ctx_) - 4) {
        state_ = FSMState::LOADED;
        std::cerr << "Error: Prompt is too long." << std::endl;
        return KLLM_ERROR_INVALID_ARG;
    }

    llama_kv_cache_clear(ctx_);

    if (llama_decode(ctx_, llama_batch_get_one(tokens_list.data(), tokens_list.size(), 0, 0))) {
        state_ = FSMState::ERROR;
        std::cerr << "Error: Initial llama_decode failed." << std::endl;
        return KLLM_ERROR_INFERENCE_FAILED;
    }

    output_text.clear();
    int n_cur = tokens_list.size();
    const int max_new_tokens = 256;

    while (n_cur < llama_n_ctx(ctx_) && n_cur < tokens_list.size() + max_new_tokens) {
        auto* logits = llama_get_logits_ith(ctx_, n_cur - 1);
        llama_token new_token_id = llama_sample_token_greedy(ctx_, nullptr, logits, 1, n_cur - 1);

        if (new_token_id == llama_token_eos(model_)) {
            break;
        }

        output_text += llama_token_to_piece(ctx_, new_token_id);

        llama_batch batch = {1, &new_token_id, nullptr, (int32_t)n_cur, 0};
        if (llama_decode(ctx_, batch)) {
            state_ = FSMState::ERROR;
            std::cerr << "Error: Subsequent llama_decode failed." << std::endl;
            return KLLM_ERROR_INFERENCE_FAILED;
        }
        n_cur++;
    }

    state_ = FSMState::LOADED;
    std::cout << "PseudoKernel: State -> LOADED (Inference complete)" << std::endl;
    return KLLM_SUCCESS;
}