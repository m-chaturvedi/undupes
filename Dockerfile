ARG IMAGE_NAME
FROM "${IMAGE_NAME}"
SHELL ["/bin/bash", "-c"]
WORKDIR /undupes
ENV INSIDE_DOCKER=1

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
        software-properties-common && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
        build-essential \
        cmake \
        fdupes \
        g++ \
        git \
        jdupes \
        jq \
        python3 \
        socat \
      && rm -rf /var/lib/apt/lists/*
