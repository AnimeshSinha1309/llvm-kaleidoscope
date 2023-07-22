FROM ubuntu:latest

# Setup the essentials in Ubuntu
ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update  
RUN apt-get upgrade
RUN apt-get install -y git-all
RUN apt-get install -y cmake
RUN apt-get install -y build-essential
RUN apt-get install -y ninja-build

# Install Boost and LLVM
WORKDIR /tmp/
RUN apt-get install -y libboost-all-dev
RUN git clone https://github.com/llvm/llvm-project.git
RUN cmake -S llvm-project/llvm -B llvm-project/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_PARALLEL_LINK_JOBS=5
RUN ninja -C llvm-project/build check-llvm

# Copy our workdir over
WORKDIR /home/llvm/
COPY . .
RUN mkdir build
RUN cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug
RUN cmake --build build -j 32

CMD ["bash"]
