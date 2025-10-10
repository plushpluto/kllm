#ifndef KLLM_H
#define KLLM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Opaque handle representing a KLLM model instance.
 */
typedef struct kllm_model kllm_model_t;

/**
 * @brief Represents the status of a KLLM operation.
 */
typedef enum {
    KLLM_SUCCESS = 0,
    KLLM_ERROR_GENERAL = 1,
    KLLM_ERROR_INVALID_ARG = 2,
    KLLM_ERROR_MODEL_LOAD_FAILED = 3,
    KLLM_ERROR_INFERENCE_FAILED = 4,
} kllm_status_t;

/**
 * @brief Initializes the KLLM subsystem.
 *
 * This function must be called before any other KLLM API functions.
 * It sets up the necessary resources and prepares the kernel layer for operation.
 *
 * @return KLLM_SUCCESS on success, or an error code otherwise.
 */
kllm_status_t kllm_initialize();

/**
 * @brief Shuts down the KLLM subsystem.
 *
 * Releases all resources allocated by the KLLM framework.
 */
void kllm_shutdown();

/**
 * @brief Loads a language model.
 *
 * @param model_path Path to the model file on the local filesystem.
 * @param[out] model_handle Pointer to a variable that will receive the model handle.
 * @return KLLM_SUCCESS on success, or an error code otherwise.
 */
kllm_status_t kllm_load_model(const char* model_path, kllm_model_t** model_handle);

/**
 * @brief Unloads a language model and releases its resources.
 *
 * @param model_handle The handle to the model to be unloaded.
 */
void kllm_unload_model(kllm_model_t* model_handle);

/**
 * @brief Runs inference on a loaded model.
 *
 * @param model_handle The handle to the model to run inference on.
 * @param input_text The input text for the model.
 * @param[out] output_buffer Buffer to store the output text.
 * @param output_buffer_size The size of the output buffer.
 * @return KLLM_SUCCESS on success, or an error code otherwise.
 */
kllm_status_t kllm_run_inference(kllm_model_t* model_handle, const char* input_text, char* output_buffer, size_t output_buffer_size);

#ifdef __cplusplus
}
#endif

#endif // KLLM_H