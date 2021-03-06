void Histogram(const float* age, int* const hist, const int n, const float group_width,
    const int m) {
#ifdef __MIC__
    const int vecLen = 16; // Length of vectorized loop (lower is better,
                           // but a multiple of 64/sizeof(int))
#else
    const int vecLen = 32; // Length of vectorized loop (lower is better,
                           // but a multiple of 64/sizeof(int))
#endif
    const float recGroupWidth = 1.0f/group_width; // Pre-compute the reciprocal
    const int nPrime = n - n%vecLen; // nPrime is a multiple of vecLen

    // Distribute work across threads
    // Strip-mining the loop in order to vectorize the inner short loop
#pragma omp parallel for
    for (int ii = 0; ii < nPrime; ii+=vecLen) { 
        // Temporary storage for vecLen indices. Necessary for vectorization
        int index[vecLen] __attribute__((aligned(64))); 

        // Vectorize the multiplication and rounding
#pragma vector aligned
        for (int i = ii; i < ii+vecLen; i++) 
            index[i-ii] = (int) ( age[i] * recGroupWidth );

        // Scattered memory access, does not get vectorized
        for (int c = 0; c < vecLen; c++) 
            // Protect the ++ operation with the atomic mutex (inefficient!)
#pragma omp atomic
            hist[index[c]]++;
    }

    // Finish with the tail of the data (if n is not a multiple of vecLen)
    for (int i = nPrime; i < n; i++)
        hist[(int) ( age[i] * recGroupWidth )]++;
}
