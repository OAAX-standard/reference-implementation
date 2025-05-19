#ifndef INTERFACE_H
#define INTERFACE_H

#include <stddef.h>
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "tensors_struct.h" // NOLINT(build/include_subdir)

#include <onnxruntime/core/session/onnxruntime_c_api.h>

/**
 * @brief This function is called only once to initialize the runtime environment.
 *
 * @return 0 if the initialization is successful, and non-zero otherwise.
 */
int runtime_initialization();

/**
 * @brief This function is called to load the model from the file path.
 *
 * @param file_path The path to the model file.
 * @return 0 if the model is loaded successfully, and non-zero otherwise.
 */
int runtime_model_loading(const char *file_path);

/**
 * @brief This function is called to store the input tensors to be processed by the runtime when it's ready.
 *
 * @note This function copies the reference of the input tensors, not the tensors themselves.
 * The runtime will free the memory of the input tensors after its processed.
 *
 * @param tensors The input tensors for the inference processing.
 *
 * @return 0 if the input tensors are stored successfully, and non-zero otherwise.
 */
int send_input(tensors_struct *input_tensors);

/**
 * @brief This function is called to retrieve any available output tensors after the inference process is done.
 *
 * @note The caller is responsible for managing the memory of the output tensors.
 *
 * @param output_tensors The output tensors of the inference process.
 *
 * @return 0 if an output is available and returned, and non-zero otherwise.
 */
int receive_output(tensors_struct **output_tensors);

/**
 * @brief This function is called to destroy the runtime environment after the inference process is stopped.
 *
 * @return 0 if the finalization is successful, and non-zero otherwise.
 */
int runtime_destruction();

/**
 * @brief This function is called to get the error message in case of a runtime error.
 *
 * @return The error message in a human-readable format. This should be allocated by the shared library, and proper deallocation should be handled by the library.
 */
const char *runtime_error_message();

/**
 * @brief This function is called to get the version of the shared library.
 *
 * @return The version of the shared library. This should be allocated by the shared library, and proper deallocation should be handled by the library.
 */
const char *runtime_version();

/**
 * @brief This function is called to get the name of the shared library.
 *
 * @return The name of the shared library. This should be allocated by the shared library, and proper deallocation should be handled by the library.
 */
const char *runtime_name();

#endif // INTERFACE_H