def cli():
    import argparse

    parser = argparse.ArgumentParser(description="Simplify any ONNX file")

    parser.add_argument("--onnx-path", required=True, help="Path to the ONNX file")
    parser.add_argument("--output-dir", required=True, help="Output directory")
    args = parser.parse_args()

    import os

    onnx_path = os.path.realpath(args.onnx_path)
    output_dir = args.output_dir

    if not onnx_path.endswith(".onnx"):
        parser.error(f"--onnx-path must point to a .onnx file, got: {onnx_path}")
    if not os.path.isfile(onnx_path):
        parser.error(f"File not found: {onnx_path}")

    from os import makedirs
    from os.path import join, split

    from .logger import Logs
    from .utils import md5_hash, simplify_onnx

    logs = Logs()
    logs.add_message(
        "Simplifying ONNX model",
        {"ONNX Path": onnx_path, "MD5": md5_hash(onnx_path), "Size (bytes)": os.path.getsize(onnx_path)},
    )

    makedirs(output_dir, exist_ok=True)

    optimized_onnx_path = simplify_onnx(onnx_path, logs)

    new_onnx_filename = split(optimized_onnx_path)[1]
    new_onnx_path = join(output_dir, new_onnx_filename)
    logs_path = join(output_dir, "logs.json")

    mime_type = "application/x-onnx; device=cpu"

    from shutil import copy

    if optimized_onnx_path.strip() != new_onnx_path.strip():
        copy(optimized_onnx_path, new_onnx_path)

    logs.add_message(
        "Successful Conversion",
        {
            "Output Directory": output_dir,
            "Output file name": new_onnx_filename,
            "MIME type": mime_type,
            "Output file MD5": md5_hash(new_onnx_path),
            "Output size (bytes)": os.path.getsize(new_onnx_path),
            "Logs file name": "logs.json",
        },
    )

    # Save after the final message so logs.json contains the complete record
    logs.save_as_json(logs_path)

    print(logs)
    print("Exiting.")
