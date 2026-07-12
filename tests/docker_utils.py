"""Shared helpers for tests that run the Docker conversion toolchain."""

import os
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).parent.parent
DOCKER_IMAGE = "oaax-cpu-toolchain:latest"

# Set OAAX_TEST_NO_CACHE=1 to ignore cached simplified models and re-convert.
NO_CACHE = os.environ.get("OAAX_TEST_NO_CACHE") == "1"


def docker_user_args() -> list:
    """Run the container as the host user: the image's appuser (uid 1000)
    can't read mode-700 tempdir mounts or write the output dir otherwise."""
    return ["--user", f"{os.getuid()}:{os.getgid()}"]


def _image_id(tag: str) -> str:
    r = subprocess.run(["docker", "images", "-q", tag], capture_output=True, text=True, timeout=5)
    return r.stdout.strip()


def check_image() -> str:
    """Return "" if the toolchain image is present and current, else the reason.

    Current means :latest is the same image as the tag for the version in the
    VERSION file — a stale :latest passes locally while CI builds fresh.
    """
    try:
        if subprocess.run(["docker", "info"], capture_output=True, timeout=5).returncode != 0:
            return "Docker is not available"
        latest = _image_id(DOCKER_IMAGE)
        if not latest:
            return f"image '{DOCKER_IMAGE}' not found — build with: bash conversion-toolchain/build-toolchain.sh"
        version = (REPO_ROOT / "VERSION").read_text().strip()
        if latest != _image_id(f"oaax-cpu-toolchain:{version}"):
            return (
                f"'{DOCKER_IMAGE}' is not the image tagged oaax-cpu-toolchain:{version} — "
                "stale image, rebuild with: bash conversion-toolchain/build-toolchain.sh"
            )
        return ""
    except Exception as e:  # docker missing, timeout, unreadable VERSION
        return f"Docker check failed: {e}"
