FROM gcc as BUILDER

RUN apt update && apt install -y cmake python3-pip git make ninja-build
RUN pip install conan && \
    conan profile new --detect default && \
        conan profile update settings.compiler.libcxx=libstdc++11 default && \
        conan profile update settings.compiler.cppstd=20 default

# Build dependencies
ARG SIMPLE_YAML=0.2.2
COPY ./simple-yaml /tmp/simple-yaml
RUN cd /tmp/simple-yaml \
    && conan create . simple-yaml/${SIMPLE_YAML}@rechip/stable -o:b enable_enum=True --build=missing

COPY logger /tmp/logger

ARG LOGGER=0.1.0
RUN cd /tmp/logger && \
        conan create . logger/${LOGGER}@rechip/experimental --build=missing

COPY conanfile.txt /tmp/source/
RUN cd /tmp/source && \
    mkdir -p build/Release && \
        cd build/Release && \
        conan install ../.. --build=missing

# Build an app

COPY CMakeLists.txt /tmp/source/
COPY src/ /tmp/source/src/
COPY include/ /tmp/source/include/

RUN cd /tmp/source && \
        cmake -DCMAKE_BUILD_TYPE=Release -B"./build/Release" && \
        cmake --build ./build/Release --config Release -j8
RUN     ls /tmp/source/build/out/Release/bin/

FROM debian:bullseye

COPY --from=BUILDER --chown=root:root /tmp/source/build/out/Release/bin/mqtt-exporter /root/mqtt-exporter
RUN chmod +x /root/mqtt-exporter
RUN ls /root/

COPY mqtt-exporter.yml /root/

WORKDIR /root
ENTRYPOINT ["/root/mqtt-exporter"]
