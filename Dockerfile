FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /ctc-simulator

COPY . .

RUN mkdir build && cd build && cmake .. && cmake --build .

EXPOSE 8080

CMD ["./bin/app"]
