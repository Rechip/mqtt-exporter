FROM debian:bookworm

RUN apt update \
    && apt install -y \
	g++-11 gcc-11 libstdc++-11-dev cmake ninja-build python3-pip \
	clang-13 clang-format-13 clang-tidy-13 libc++-13-dev \
	bash-completion valgrind \
	&& pip install conan
RUN apt install -y ca-certificates

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 60 \
	&& update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 60 \
	&& update-alternatives --install /usr/bin/clang clang /usr/bin/clang-13 60 \
	&& update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-13 60 \
	&& update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-13 60 \
	&& update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-13 60

RUN useradd -m -s /bin/bash vscode
USER vscode

RUN conan profile new --detect default \
	&& conan profile update settings.compiler.libcxx=libstdc++11 default \
	&& conan profile update settings.compiler.cppstd=20 default

ENTRYPOINT [ "/bin/bash" ]

