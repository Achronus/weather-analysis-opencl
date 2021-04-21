kernel void minReduce(global const int* input, global int* results, local int *scratch)
{
  // Initalize variables
  int gid = get_global_id(0);
  int lid = get_local_id(0);
  int N = get_local_size(0);

  // Cache all values from global to local memory
  scratch[lid] = input[gid];

  // Wait for local memory to be copied
  barrier(CLK_LOCAL_MEM_FENCE);

  // Calculate min value
  for (int i = 1; i < N; i *= 2)
  {
    if ((lid % (i * 2) == 0) && ((lid + i) < N))
    {
      if (scratch[lid + i] < scratch[lid])
      {
        scratch[lid] = scratch[lid + i];
      }
    }

    // Wait for sync
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  // Set result for the statistic
  if (lid == 0)
  {
    atomic_min(results, scratch[lid]);
  }
}

kernel void maxReduce(global const int* input, global int* results, local int *scratch)
{
  // Initalize variables
  int gid = get_global_id(0);
  int lid = get_local_id(0);
  int N = get_local_size(0);

  // Cache all values from global to local memory
  scratch[lid] = input[gid];

  // Wait for local memory to be copied
  barrier(CLK_LOCAL_MEM_FENCE);

  // Calculate max value
  for (int i = 1; i < N; i *= 2)
  {
    if ((lid % (i * 2) == 0) && ((lid + i) < N))
    {
      if (scratch[lid + i] > scratch[lid])
      {
        scratch[lid] = scratch[lid + i];
      }
    }

    // Wait for sync
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  // Set result for the statistic
  if (lid == 0)
  {
    atomic_max(results, scratch[lid]);
  }
}

kernel void sumReduce(global int const* input, global int* results, local int *scratch) {
	// Initalize variables
  int gid = get_global_id(0);
  int lid = get_local_id(0);
  int N = get_local_size(0);

  // Cache all values from global to local memory
  scratch[lid] = input[gid];

  // Wait for local memory to be copied
  barrier(CLK_LOCAL_MEM_FENCE);

  // Calculate sum
  for (int i = 1; i < N; i *= 2)
  {
    if ((lid % (i * 2) == 0) && ((lid + i) < N))
    {
      scratch[lid] += scratch[lid + i];
    }

    // Wait for sync
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  // Set result for the statistic
  if (lid == 0)
  {
    atomic_add(results, scratch[lid]);
  }
}

kernel void varianceReduce(global int const* input, global int* results, local int *scratch, int mean, int dataSize) {
	// Initalize variables
  int gid = get_global_id(0);
  int lid = get_local_id(0);
  int N = get_local_size(0);

  // Ignore padded values
  if (gid < dataSize)
  {
    // Cache all values from global to local memory and perform variance calculation
    scratch[lid] = (input[gid] - mean) * (input[gid] - mean);

    // Wait for local memory to be copied
    barrier(CLK_LOCAL_MEM_FENCE);

    // Calculate sum
    for (int i = 1; i < N; i *= 2)
    {
      if ((lid % (i * 2) == 0) && ((lid + i) < N))
      {
        scratch[lid] += scratch[lid + i];
      }

      // Wait for sync
      barrier(CLK_LOCAL_MEM_FENCE);
    }

    // Convert value back to normal (100 ^ 2 = 10,000)
    scratch[lid] = scratch[lid] / 10000.0;

    // Set result for the statistic
    if (lid == 0)
    {
      atomic_add(results, scratch[lid]);
    }
  }
}

// Parallel Selection Sort using global memory
// ref - http://www.bealto.com/gpu-sorting_parallel-selection.html
kernel void selectionSort(global const int* input, global int* output)
{
  int gid = get_global_id(0);
  int N = get_global_size(0);
  int pos = 0;
  int iData = input[gid];

  for (int j = 0; j < N; j++)
  {
    int jData = input[j];
    bool smallest = ((jData < iData) || (jData == iData && j < gid));
    pos += (smallest) ? 1 : 0;
  }

  // Store to output
  output[pos] = iData;
}