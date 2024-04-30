#include "runtime_utils.h"

extern const OrtApi *api;
extern OrtSession *session;
extern OrtAllocator *allocator;

int runtime_core_process_status(OrtStatus *status) {
    if (status == NULL) return 0;
    printf("Error: RUNTIME - Message by ORT API: \n%s\n", api->GetErrorMessage(status));;
    api->ReleaseStatus(status);
    return 1;
}

char **runtime_core_get_input_names(int32_t *input_names_count, int32_t **input_data_types) {
    printf("Notice: RUNTIME - Reading input names and data types from ONNX file\n");

    size_t s_input_count;
    int32_t input_count;

    runtime_core_process_status(api->SessionGetInputCount(session, &s_input_count));
    input_count = (int32_t) s_input_count;

    char **tmp_input_names = malloc(sizeof(char *) * input_count);
    char **input_names = malloc(sizeof(char *) * input_count);
    if (input_data_types != NULL)
        input_data_types[0] = malloc(sizeof(int32_t) * input_count);

    for (int i = 0; i < input_count; i++) {
        if (input_data_types != NULL) {
            OrtTypeInfo *type_info;
            OrtTensorTypeAndShapeInfo *type_shape_info;
            ONNXTensorElementDataType onnx_type;
            runtime_core_process_status(api->SessionGetInputTypeInfo(session, i, &type_info));
            runtime_core_process_status(api->CastTypeInfoToTensorInfo(type_info,
                                                                      (const OrtTensorTypeAndShapeInfo **) &type_shape_info));
            runtime_core_process_status(api->GetTensorElementType(type_shape_info, &onnx_type));
            input_data_types[0][i] = (int) onnx_type;
            api->ReleaseTypeInfo(type_info);
        }

        runtime_core_process_status(api->SessionGetInputName(session, i, allocator, tmp_input_names + i));
        input_names[i] = malloc(sizeof(char) * strlen(tmp_input_names[i]));
        strcpy(input_names[i], tmp_input_names[i]);
        runtime_core_process_status(api->AllocatorFree(allocator, tmp_input_names[i]));
    }

    free(tmp_input_names);
    *input_names_count = input_count;

    return input_names;
}


char **runtime_core_get_output_names(int32_t *output_names_count, int32_t **output_data_types) {
    printf("Notice: RUNTIME - Reading output names and data types from ONNX file\n");

    size_t s_output_count;
    int32_t output_count;

    runtime_core_process_status(api->SessionGetOutputCount(session, &s_output_count));
    output_count = (int32_t) s_output_count;

    char **tmp_output_names = malloc(sizeof(char *) * output_count);
    char **output_names = malloc(sizeof(char *) * output_count);
    if (output_data_types != NULL)
        output_data_types[0] = malloc(sizeof(int32_t) * output_count);

    for (int i = 0; i < output_count; i++) {
        if (output_data_types != NULL) {
            OrtTypeInfo *type_info;
            OrtTensorTypeAndShapeInfo *type_shape_info;
            ONNXTensorElementDataType onnx_type;
            runtime_core_process_status(api->SessionGetOutputTypeInfo(session, i, &type_info));
            runtime_core_process_status(api->CastTypeInfoToTensorInfo(type_info,
                                                                      (const OrtTensorTypeAndShapeInfo **) &type_shape_info));
            runtime_core_process_status(api->GetTensorElementType(type_shape_info, &onnx_type));
            output_data_types[0][i] = (int) onnx_type;
            api->ReleaseTypeInfo(type_info);
        }

        runtime_core_process_status(api->SessionGetOutputName(session, i, allocator, tmp_output_names + i));
        output_names[i] = malloc(sizeof(char) * strlen(tmp_output_names[i]));
        strcpy(output_names[i], tmp_output_names[i]);
        runtime_core_process_status(api->AllocatorFree(allocator, tmp_output_names[i]));
    }

    free(tmp_output_names);
    *output_names_count = output_count;

    return output_names;
}


int64_t runtime_util_get_sizeof_onnx_type(int32_t datatype) {
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__INT8)
        return sizeof(int8_t);
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__UINT8)
        return sizeof(uint8_t);
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__BOOL)
        return sizeof(bool);
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__INT16)
        return sizeof(int16_t);
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__INT16)
        return sizeof(int16_t);
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__UINT16)
        return sizeof(uint16_t);
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__INT32)
        return sizeof(int32_t);
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__INT64)
        return sizeof(int64_t);
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__FLOAT)
        return sizeof(float);
    if (datatype == ONNX__TENSOR_PROTO__DATA_TYPE__DOUBLE)
        return sizeof(double);
    return 0;
}
