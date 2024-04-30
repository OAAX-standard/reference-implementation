#include "runtime_core.h"
#include "runtime_utils.h"

#ifndef ONNXRUNTIME_API_VERSION
#define ONNXRUNTIME_API_VERSION 15
#endif

#define RUNTIME_ORT_CORE_EXEC(return_code, error) ({int32_t code = return_code; if (code != 0) {return code;}})

const OrtApi *api;
OrtSession *session;
OrtAllocator *allocator;
OrtMemoryInfo *memory_info;
OrtRunOptions *run_options;
OrtEnv *env;
OrtSessionOptions *session_options;
int return_code;
// Inference results tensors
tensors_struct local_output_tensors = {0, NULL, NULL, NULL, NULL, NULL};

int runtime_initialization() {
    printf("Notice: RUNTIME - Initializing OnnxRuntime\n");
    long number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);

    // get the correct api version
    api = OrtGetApiBase()->GetApi(ONNXRUNTIME_API_VERSION);

    // create the environment
    runtime_core_process_status(api->CreateEnv(ORT_LOGGING_LEVEL_FATAL, "ORT_LOGGER", &env));

    // Create session options
    runtime_core_process_status(api->CreateSessionOptions(&session_options));

    // choices: ORT_DISABLE_ALL, ORT_ENABLE_BASIC, ORT_ENABLE_EXTENDED, ORT_ENABLE_ALL
    runtime_core_process_status(api->SetSessionGraphOptimizationLevel(session_options, ORT_ENABLE_ALL));

    // Divide threads equally between runtimes
    const int max_threads_per_runtime = 8;
    int interop_threads = (int) number_of_processors;

    // Clamp value between 1 and max_threads_per_runtime
    if (interop_threads < 1) {
        interop_threads = 1;
    } else if (interop_threads > max_threads_per_runtime) {
        interop_threads = max_threads_per_runtime;
    }

    runtime_core_process_status(api->SetIntraOpNumThreads(session_options, interop_threads));
    runtime_core_process_status(api->SetInterOpNumThreads(session_options, 1));
    runtime_core_process_status(api->SetSessionExecutionMode(session_options, ORT_PARALLEL));

    char **providers;
    int number_providers;
    runtime_core_process_status(api->GetAvailableProviders(&providers, &number_providers));
    for (int i = 0; i < number_providers; ++i)
        printf("Notice: RUNTIME - Provider id: %i - name: %s\n", i, providers[i]);
    runtime_core_process_status(api->ReleaseAvailableProviders(providers, number_providers));

    // create run options
    runtime_core_process_status(api->CreateRunOptions(&run_options));

    return 0;
}

int runtime_model_loading(const char *file_path) {
    printf("Notice: RUNTIME - Reading ONNX file from '%s' ...\n", file_path);

    // Create a session
    runtime_core_process_status(api->CreateSession(env, file_path, session_options, &session));

    // create allocator
    runtime_core_process_status(api->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info));
    runtime_core_process_status(api->CreateAllocator(session, memory_info, &allocator));

    return 0;
}

int runtime_inference_execution(tensors_struct *input_tensors, tensors_struct *output_tensors) {
    printf("Notice: RUNTIME - start core exec.\n");
    int error = 0;
    int number_inputs = (int) input_tensors->num_tensors;
    uint8_t **inputs = (uint8_t **) input_tensors->data;
    long int **input_shapes = (long int **) input_tensors->shapes;
    size_t *input_ranks = (size_t *) input_tensors->ranks;
    size_t *input_sizes = (size_t *) malloc(sizeof(size_t) * number_inputs);

    for (int i = 0; i < number_inputs; i++) {
        input_sizes[i] = 1;
        for (size_t j = 0; j < input_ranks[i]; j++)
            input_sizes[i] *= input_shapes[i][j];
    }

    int number_outputs;
    int *input_dtypes, *output_dtypes;

    printf("Notice: RUNTIME - start reading input names.\n");
    char **input_names = runtime_core_get_input_names(&number_inputs, &input_dtypes);

    // Make sure that the number of input tensors is the same as the one passed to the runtime
    if (number_inputs != (int) input_tensors->num_tensors) {
        printf("Error: RUNTIME - The number of input tensors does not match the number of input tensors in the model.\n");
        printf("Expected: %d, Got: %zu\n", number_inputs, input_tensors->num_tensors);
        free(input_dtypes);
        free(input_sizes);
        for (int i = 0; i < number_inputs; i++) free(input_names[i]);
        free(input_names);
        return 1;
    }

    printf("Notice: RUNTIME - start reading output names.\n");

    char **output_names = runtime_core_get_output_names(&number_outputs, &output_dtypes);

    printf("Notice: RUNTIME - runtime_core_parse_input_data running.\n");

    // run model
    OrtValue **input_values = malloc(sizeof(OrtValue *) * number_inputs);
    OrtValue **output_values = malloc(sizeof(OrtValue *) * number_outputs);

    printf("Notice: RUNTIME - building input tensors for onnxruntime ...\n");
    for (int i = 0; i < number_inputs; ++i) {
        RUNTIME_ORT_CORE_EXEC(runtime_core_process_status(
                api->CreateTensorWithDataAsOrtValue(memory_info,
                                                    inputs[i],
                                                    input_sizes[i] * runtime_util_get_sizeof_onnx_type(input_dtypes[i]),
                                                    input_shapes[i],
                                                    input_ranks[i],
                                                    input_dtypes[i],
                                                    input_values + i
                )), &error);

    }
    for (int i = 0; i < number_outputs; ++i)
        output_values[i] = NULL;

    printf("Notice: RUNTIME - onnxruntime inferring ...\n");
    RUNTIME_ORT_CORE_EXEC(runtime_core_process_status(api->Run(session,
                                                               run_options,
                                                               (const char *const *) input_names,
                                                               (const OrtValue *const *) input_values,
                                                               number_inputs,
                                                               (const char *const *) output_names,
                                                               number_outputs,
                                                               output_values)), &error);

    printf("Notice: RUNTIME - cleaning up inputs ...\n");
    // clean up
    // Inputs
    for (int i = 0; i < number_inputs; i++) {
        free(input_names[i]);
        api->ReleaseValue(input_values[i]);
    }
    free(input_dtypes);
    free(input_sizes);
    free(input_values);

    void **outputs = malloc(sizeof(void *) * number_outputs);
    size_t **output_shapes = malloc(sizeof(size_t *) * number_outputs);
    size_t *output_ranks = malloc(sizeof(size_t) * number_outputs);

    printf("Notice: RUNTIME - reading output tensors ...\n");
    for (int i = 0; i < number_outputs; ++i) {
        OrtTensorTypeAndShapeInfo *type_shape;

        // get shape information
        RUNTIME_ORT_CORE_EXEC(runtime_core_process_status(api->GetTensorTypeAndShape(output_values[i], &type_shape)),
                              &error);

        // get output size
        size_t size;
        RUNTIME_ORT_CORE_EXEC(runtime_core_process_status(api->GetTensorShapeElementCount(type_shape, &size)), &error);

        // get output rank
        size_t rank;
        RUNTIME_ORT_CORE_EXEC(runtime_core_process_status(api->GetDimensionsCount(type_shape, &rank)), &error);
        output_ranks[i] = rank;

        // get output shape
        output_shapes[i] = malloc(sizeof(int64_t) * rank);
        RUNTIME_ORT_CORE_EXEC(runtime_core_process_status(api->GetDimensions(type_shape, (int64_t *) output_shapes[i], rank)),
                              &error);

        // get output value
        size_t output_bytes = size * runtime_util_get_sizeof_onnx_type(output_dtypes[i]);
        outputs[i] = malloc(output_bytes);
        void *tmp;
        RUNTIME_ORT_CORE_EXEC(runtime_core_process_status(api->GetTensorMutableData(output_values[i], &tmp)), &error);
        memcpy(outputs[i], tmp, output_bytes);

        api->ReleaseTensorTypeAndShapeInfo(type_shape);
        api->ReleaseValue(output_values[i]);
    }
    free(output_values);

    // Assign the output tensors to the output_tensors pointer
    local_output_tensors.num_tensors = number_outputs;
    local_output_tensors.ranks = output_ranks;
    local_output_tensors.shapes = output_shapes;
    local_output_tensors.data_types = (tensor_data_type *) output_dtypes;
    local_output_tensors.data = outputs;
    local_output_tensors.names = output_names;
    *output_tensors = local_output_tensors;

    return error;
}

int runtime_inference_cleanup() {
    printf("Notice: RUNTIME - Freeing output tensors returned by the runtime ...\n");

    if (local_output_tensors.num_tensors > 0) {
        for (size_t i = 0; i < local_output_tensors.num_tensors; i++) {
            free(local_output_tensors.shapes[i]);
            free(local_output_tensors.data[i]);
        }
        free(local_output_tensors.ranks);
        free(local_output_tensors.data_types);
        free(local_output_tensors.shapes);
        free(local_output_tensors.data);

        // Free the names if they are not NULL
        if (local_output_tensors.names != NULL) {
            for (size_t i = 0; i < local_output_tensors.num_tensors; i++) {
                free(local_output_tensors.names[i]);
            }
            free(local_output_tensors.names);
        }
    } else {
        return 1;
    }

    return 0;
}

int runtime_destruction() {
    printf("Notice: RUNTIME - Releasing all objects created by ORT API ...\n");

    api->ReleaseRunOptions(run_options);
    api->ReleaseMemoryInfo(memory_info);
    api->ReleaseAllocator(allocator);
    api->ReleaseEnv(env);
    api->ReleaseSession(session);
    api->ReleaseSessionOptions(session_options);
    return 0;
}

const char *runtime_error_message() {
    return "Check the stdout for the error message.";
}

const char *runtime_version() {
    return "0.1.0";
}

const char *runtime_name() {
    return "OnnxRuntime";
}
