#include "functional.h"

float relu(float x)
{
    // TODO: Implement relu
}

void applyRelu(float *input, int inputSize)
{
    for (int i = 0; i < inputSize; i++)
    {
        input[i] = relu(input[i]);
    }
}

float *softmax(float *input, int inputSize)
{
    // TODO: Implement softmax

    // Find maximum of input vector

    // Compute exp of input - maxInput to avoid underflow

    // Normalise and apply log
}