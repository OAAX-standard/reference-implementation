ARG DEBIAN_FRONTEND=noninteractive
FROM python:3.8.16 as base

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# hadolint ignore=DL3008
RUN apt-get update -qq -y --fix-missing \
  && apt-get install -qq -y --no-install-recommends \
    gcc g++ curl make xxd wget apt-utils software-properties-common git autoconf \
    automake libtool python3 unzip cmake build-essential protobuf-compiler libprotoc-dev \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY requirements.txt /app/requirements.txt
COPY setup.py /app/setup.py
COPY conversion_toolchain /app/conversion_toolchain
COPY scripts/ /app/scripts

RUN pip install --no-cache-dir -r requirements.txt

ENTRYPOINT ["bash", "/app/scripts/convert.sh"]
