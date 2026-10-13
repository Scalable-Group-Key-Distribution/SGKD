# SGKD Protocol & Cryptographic Benchmark

## Overview

This project contains implementations for cryptographic primitives and benchmark the Scalable Group Key Distribution (SGKD) protocol. Specifically, it includes:

- **Primitives-Benchmark**
  - `primitives-benchmark.cpp`: Benchmarks various cryptographic operations using OpenSSL.
  - `pairing-benchmark.cpp`: Benchmarks pairing-based cryptographic operations using Relic.
- **SGKD-Protocol**
  - `ta.cpp`: Implements the Trusted Authority (TA) component of the SGKD protocol.
  - `vehicle.cpp`: Implements the vehicle-side operations of the SGKD protocol.
  - `utils.cpp`: Provides utility functions and shared code used by other SGKD protocol components.

## Project Structure

```
SGKD/
├── Primitives-Benchmark/
│   ├── primitives-benchmark.cpp
│   └── pairing-benchmark.cpp
├── SGKD-Protocol/
│   ├── ta.cpp
│   ├── vehicle.cpp
|   └── utils.cpp
└── Makefile
```
## Dependencies

- C++17 or newer
- OpenSSL development libraries (`libssl-dev`)
- RELIC toolkit (for pairing-based cryptography)

### Installing OpenSSL

On Ubuntu/Debian, install OpenSSL development libraries with:

```bash
sudo apt-get update
sudo apt-get install libssl-dev
```

### Installing RELIC

To install the RELIC toolkit:

```bash
sudo apt-get install cmake libgmp-dev libssl-dev
git clone https://github.com/relic-toolkit/relic.git
cd relic
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local/relic ..
make
sudo make install
```

Make sure to adjust the `CMAKE_INSTALL_PREFIX` if you want to install RELIC elsewhere.  
The Makefile assumes RELIC is available at `/usr/local/relic`.

## Building the Project

Ensure you have `g++`, OpenSSL and Relic installed. To build all executables, run:

```bash
make
```

This will generate the following executables in the project directory:

- `primitives-benchmark`
- `pairing-benchmark`
- `ta`
- `vehicle`

## Running the Executables

Run each executable as follows:

```bash
./primitives-benchmark
./pairing-benchmark
./ta
./vehicle
```

Each program will display its respective output and benchmark results.

## Cleaning Up

To remove all compiled executables, run:

```bash
make clean
```

This will delete all generated binaries from the project directory.


## License

This project is provided for academic and research purposes.