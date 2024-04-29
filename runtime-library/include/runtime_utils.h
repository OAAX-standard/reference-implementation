#ifndef RUNTIME_C_RUNTIMES_ORT_CORE_INCLUDE_RUNTIME_RUNTIME_UTILS_H_
#define RUNTIME_C_RUNTIMES_ORT_CORE_INCLUDE_RUNTIME_RUNTIME_UTILS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "onnx.pb-c.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <onnxruntime/core/session/onnxruntime_c_api.h>

/**
 * @brief Process the status returned by the ONNX Runtime API. Print the error message if the status is not successful.
 * @param status The status returned by the ONNX Runtime API.
 * @return 0 if the status is successful, and non-zero otherwise.
 */
int runtime_core_process_status(OrtStatus *status);

/**
 * @brief Retrieve number of inputs, each input names and its corresponding data type from the ONNX file.
 * @param [out] input_names_count Number of inputs
 * @param [out] input_data_types Array of inputs data type based on the onnx.pb-c.h._Onnx__TensorProto__DataType enum
 * @return Array of input names
 */
char **runtime_core_get_input_names(int32_t *input_names_count, int32_t **input_data_types);

/**
 * @brief Retrieve number of outputs, each output names and its corresponding data type from the ONNX file.
 * @param output_names_count [out] Number of outputs
 * @param output_data_types [out] Array of outputs data type based on the onnx.pb-c.h._Onnx__TensorProto__DataType enum
 * @return Array of output names
 */
char **runtime_core_get_output_names(int32_t *output_names_count, int32_t **output_data_types);

int64_t runtime_util_get_sizeof_onnx_type(int32_t datatype);

#endif //RUNTIME_C_RUNTIMES_ORT_CORE_INCLUDE_RUNTIME_RUNTIME_UTILS_H_
