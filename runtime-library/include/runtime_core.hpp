
#ifndef RUNTIME_CORE_HPP
#define RUNTIME_CORE_HPP

#include "tensors_struct.h"

#ifdef _WIN32
#define EXPOSE_FUNCTION __declspec(dllexport)
#else
#define EXPOSE_FUNCTION __attribute__((visibility("default")))
#endif

EXPOSE_FUNCTION extern "C" int runtime_initialization_with_args(int length, char **keys, void **values);

EXPOSE_FUNCTION extern "C" int runtime_initialization();

EXPOSE_FUNCTION extern "C" int runtime_model_loading(const char *model_path);

EXPOSE_FUNCTION extern "C" int send_input(tensors_struct *input_tensors);

EXPOSE_FUNCTION extern "C" int receive_output(tensors_struct **output_tensors);

EXPOSE_FUNCTION extern "C" int runtime_destruction();

EXPOSE_FUNCTION const char *runtime_error_message();

EXPOSE_FUNCTION const char *runtime_version();

EXPOSE_FUNCTION const char *runtime_name();

#endif // RUNTIME_CORE_HPP