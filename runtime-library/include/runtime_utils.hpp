#ifndef RUNTIME_UTILS_HPP
#define RUNTIME_UTILS_HPP

#include <onnxruntime_cxx_api.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

#include "oaax_runtime.h"

ONNXTensorElementDataType map_to_ort_type(TensorElementType t);
TensorElementType map_from_ort_type(ONNXTensorElementDataType type);
size_t get_element_byte_size(TensorElementType t);

std::vector<std::string> get_input_names(Ort::Session& session);
std::vector<std::string> get_output_names(Ort::Session& session);

std::shared_ptr<spdlog::logger> initialize_logger(const std::string& log_file, int file_level = spdlog::level::info,
                                                  int console_level = spdlog::level::info,
                                                  const std::string prefix = "OAAX");

void destroy_logger(std::shared_ptr<spdlog::logger> logger);

#endif  // RUNTIME_UTILS_HPP
