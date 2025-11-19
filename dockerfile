FROM ubuntu:22.04 AS builder


# Install required build tools
# Install required build tools
RUN apt update && apt install -y \
    build-essential \
    cmake \
    git \
    curl \
    pkg-config \
    zip \
    unzip \
    tar \
    sqlite3 \
    libsqlite3-dev \
    autoconf \
    automake \
    libtool


# ---- Install vcpkg ----
WORKDIR /opt
RUN git clone https://github.com/microsoft/vcpkg.git
RUN ./vcpkg/bootstrap-vcpkg.sh

ENV VCPKG_ROOT=/opt/vcpkg
ENV CMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake

# Install project dependencies
RUN /opt/vcpkg/vcpkg install crow:x64-linux
RUN /opt/vcpkg/vcpkg install sqlite3:x64-linux
RUN /opt/vcpkg/vcpkg install libsodium:x64-linux  


# ---- Copy project source ----
WORKDIR /app
COPY . .

# ---- Build project ----
RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DCMAKE_CXX_STANDARD=17

RUN cmake --build build --config Release


# ============================
# 2) RUNTIME STAGE
# ============================
FROM ubuntu:22.04 AS runtime

RUN apt update && apt install -y sqlite3 libsqlite3-0 && apt clean

WORKDIR /app

# Copy built executable and assets
COPY --from=builder /app/build/fitness ./fitness-app
COPY --from=builder /app/code/frontend ./code/frontend    
COPY --from=builder /app/code/backend/fitness.db ./code/backend/fitness.db


# Expose Crow port
EXPOSE 18080

CMD ["./fitness-app"]
