#include "runtime_utils.hpp"

#include <onnxruntime_cxx_api.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <stdexcept>

ONNXTensorElementDataType map_to_ort_type(TensorElementType t) {
    switch (t) {
        case DATA_TYPE_FLOAT:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
        case DATA_TYPE_UINT8:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8;
        case DATA_TYPE_INT8:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8;
        case DATA_TYPE_UINT16:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16;
        case DATA_TYPE_INT16:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16;
        case DATA_TYPE_INT32:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
        case DATA_TYPE_INT64:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64;
        case DATA_TYPE_STRING:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING;
        case DATA_TYPE_BOOL:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL;
        case DATA_TYPE_FLOAT16:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;
        case DATA_TYPE_DOUBLE:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE;
        case DATA_TYPE_UINT32:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32;
        case DATA_TYPE_UINT64:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64;
        case DATA_TYPE_COMPLEX64:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_COMPLEX64;
        case DATA_TYPE_COMPLEX128:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_COMPLEX128;
        case DATA_TYPE_BFLOAT16:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_BFLOAT16;
        case DATA_TYPE_FLOAT8E4M3FN:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT8E4M3FN;
        case DATA_TYPE_FLOAT8E4M3FNUZ:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT8E4M3FNUZ;
        case DATA_TYPE_FLOAT8E5M2:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT8E5M2;
        case DATA_TYPE_FLOAT8E5M2FNUZ:
            return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT8E5M2FNUZ;
        default:
            throw std::runtime_error("Unsupported or unmapped TensorElementType: " +
                                     std::to_string(static_cast<int>(t)));
    }
}

TensorElementType map_from_ort_type(ONNXTensorElementDataType type) {
    switch (type) {
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
            return DATA_TYPE_FLOAT;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8:
            return DATA_TYPE_UINT8;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8:
            return DATA_TYPE_INT8;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16:
            return DATA_TYPE_UINT16;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16:
            return DATA_TYPE_INT16;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
            return DATA_TYPE_INT32;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
            return DATA_TYPE_INT64;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING:
            return DATA_TYPE_STRING;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
            return DATA_TYPE_BOOL;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16:
            return DATA_TYPE_FLOAT16;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
            return DATA_TYPE_DOUBLE;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32:
            return DATA_TYPE_UINT32;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64:
            return DATA_TYPE_UINT64;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_COMPLEX64:
            return DATA_TYPE_COMPLEX64;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_COMPLEX128:
            return DATA_TYPE_COMPLEX128;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_BFLOAT16:
            return DATA_TYPE_BFLOAT16;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT8E4M3FN:
            return DATA_TYPE_FLOAT8E4M3FN;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT8E4M3FNUZ:
            return DATA_TYPE_FLOAT8E4M3FNUZ;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT8E5M2:
            return DATA_TYPE_FLOAT8E5M2;
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT8E5M2FNUZ:
            return DATA_TYPE_FLOAT8E5M2FNUZ;
        default:
            throw std::runtime_error("Unsupported ORT element type: " + std::to_string(static_cast<int>(type)));
    }
}

size_t get_element_byte_size(TensorElementType t) {
    switch (t) {
        case DATA_TYPE_FLOAT:
            return 4;
        case DATA_TYPE_FLOAT16:
            return 2;
        case DATA_TYPE_BFLOAT16:
            return 2;
        case DATA_TYPE_DOUBLE:
            return 8;
        case DATA_TYPE_INT8:
            return 1;
        case DATA_TYPE_INT16:
            return 2;
        case DATA_TYPE_INT32:
            return 4;
        case DATA_TYPE_INT64:
            return 8;
        case DATA_TYPE_UINT8:
            return 1;
        case DATA_TYPE_UINT16:
            return 2;
        case DATA_TYPE_UINT32:
            return 4;
        case DATA_TYPE_UINT64:
            return 8;
        case DATA_TYPE_BOOL:
            return 1;
        case DATA_TYPE_COMPLEX64:
            return 8;
        case DATA_TYPE_COMPLEX128:
            return 16;
        case DATA_TYPE_FLOAT8E4M3FN:
            return 1;
        case DATA_TYPE_FLOAT8E4M3FNUZ:
            return 1;
        case DATA_TYPE_FLOAT8E5M2:
            return 1;
        case DATA_TYPE_FLOAT8E5M2FNUZ:
            return 1;
        default:
            return 0;
    }
}

std::vector<std::string> get_input_names(Ort::Session& session) {
    Ort::AllocatorWithDefaultOptions alloc;
    size_t n = session.GetInputCount();
    std::vector<std::string> names;
    names.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        auto name = session.GetInputNameAllocated(i, alloc);
        names.emplace_back(name.get());
    }
    return names;
}

std::vector<std::string> get_output_names(Ort::Session& session) {
    Ort::AllocatorWithDefaultOptions alloc;
    size_t n = session.GetOutputCount();
    std::vector<std::string> names;
    names.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        auto name = session.GetOutputNameAllocated(i, alloc);
        names.emplace_back(name.get());
    }
    return names;
}

std::shared_ptr<spdlog::logger> initialize_logger(const std::string& log_file, int file_level, int console_level,
                                                  const std::string prefix) {
    try {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(static_cast<spdlog::level::level_enum>(console_level));

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(log_file, 1024 * 1024 * 5, 3);
        file_sink->set_level(static_cast<spdlog::level::level_enum>(file_level));

        static auto thread_pool = std::make_shared<spdlog::details::thread_pool>(8192, 1);

        auto logger =
            std::make_shared<spdlog::async_logger>(prefix, spdlog::sinks_init_list{console_sink, file_sink},
                                                   thread_pool, spdlog::async_overflow_policy::overrun_oldest);

        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [" + prefix + "] [%^%l%$] %v");
        logger->set_level(static_cast<spdlog::level::level_enum>(std::min(file_level, console_level)));
        logger->flush_on(spdlog::level::trace);

        return logger;
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Logger initialization failed: " << ex.what() << "\n";
        exit(EXIT_FAILURE);
    }
}

void destroy_logger(std::shared_ptr<spdlog::logger> logger) {
    if (logger) {
        logger->flush();
        spdlog::drop(logger->name());
        logger = nullptr;
    }
}
