#include "functional.h"

float relu(float x)
{
    // TODO: Implement relu
    if (x < 0) {
        return 0;
    } else {
        return x;
    }
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
    // Find maximum of input vector
    float maxInput = input[0];
    for (int i = 0; i < inputSize; i++){
        if (input[i] > maxInput){
            maxInput = input[i];
        }
    }

    // Compute exp of input - maxInput to avoid underflow
    float exp_sum = 0.00f;
    for (int i = 0; i < inputSize; i++){
        exp_sum += expf(input[i]-maxInput);
    }

    // Normalise and apply log
    float log_sum = logf(exp_sum);
    float *output = malloc(sizeof(float)*inputSize);
    for (int i = 0; i < inputSize; i++){
        output[i] = input[i] - maxInput - log_sum;
    }

    return output;
}