
#ifndef RUNTIME_CORE_HPP
#define RUNTIME_CORE_HPP

#include "tensors_struct.h"

extern "C" int runtime_initialization_with_args(int length, char **keys, void **values);

extern "C" int runtime_initialization();

extern "C" int runtime_model_loading(const char *model_path);

extern "C" int send_input(tensors_struct *input_tensors);

extern "C" int receive_output(tensors_struct **output_tensors);

extern "C" int runtime_destruction();

extern "C" const char *runtime_error_message();

extern "C" const char *runtime_version();

extern "C" const char *runtime_name();

#endif // RUNTIME_CORE_HPP