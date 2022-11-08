#include "soro/infrastructure/gpu/exclusion.h"

#include <cstdio>

namespace soro::infrastructure {

__global__ void hello_world() { printf("Hello World from the GPU!\n"); }

void hello_world_cpu() {
  hello_world<<<1, 1>>>();
  cudaDeviceSynchronize();
}

}  // namespace soro::infrastructure