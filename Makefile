
# Makefile for SGKD Protocol and Primitives Benchmarking
# This Makefile compiles the necessary C++ source files into executables.
# It assumes the RELIC library is installed and available in the specified paths.
# Usage: Run 'make' to compile all executables, 'make clean' to remove them.
# Variables
# CXX: The C++ compiler to use
# CXXFLAGS: Compiler flags for C++ compilation
# LDFLAGS: Linker flags for linking with libraries
# Executable names: Names of the output binaries
# Source files: Paths to the source files to be compiled
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
LDFLAGS = -lcrypto -lssl

# Executable names
PBENCH = primitives-benchmark
PAIRBENCH = pairing-benchmark
TA = ta
VEHICLES = vehicle
# Source files
PBENCH_SRC = Primitives-Benchmark/primitives-benchmark.cpp
PAIRBENCH_SRC = Primitives-Benchmark/pairing-benchmark.cpp
TA_SRC = SGKD-Protocol/ta.cpp
VEHICLES_SRC = SGKD-Protocol/vehicle.cpp
# Targets
# The 'all' target builds all executables
# Each executable has its own target that compiles the corresponding source file
all: $(PBENCH) $(PAIRBENCH) $(TA) $(VEHICLES)

$(PBENCH): $(PBENCH_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(PAIRBENCH): $(PAIRBENCH_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lrelic -lgmp

$(TA): $(TA_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ -I/usr/local/relic/include -L/usr/local/relic/lib -lrelic_s -lssl -lcrypto

$(VEHICLES): $(VEHICLES_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ -I/usr/local/relic/include -L/usr/local/relic/lib -lrelic_s -lssl -lcrypto

# The 'clean' target removes all executables
clean:
	rm -f $(PBENCH) $(PAIRBENCH) $(TA) $(VEHICLES)
