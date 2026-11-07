#include "linear.h"

float *linear(float *input, float **weights, float *biases, int inputSize, int outputSize)
{
    // TODO: Implement the linear function (fully connected layer)
    if (inputSize < 0 || outputSize <= 0 || biases == NULL) {
        return NULL;
    }

    // Input and weights are only needed for non-empty input.
    if (inputSize > 0 && (input == NULL || weights == NULL)) {
        return NULL;
    }

    float *output = malloc(sizeof(float) * outputSize);
    for (int i = 0; i < outputSize; i++) {
        output[i] = 0.00f;
        for (int j = 0; j < inputSize; j++) {
            output[i] += weights[i][j] * input[j];
        }
        output[i] += biases[i];
    }
    return output;
}
