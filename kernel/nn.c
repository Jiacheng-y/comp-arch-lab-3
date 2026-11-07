#include "nn.h"

float *flatten(float ***input, int inputSize, int depth)
{
  // TODO: Implement the flatten function
  if (input == NULL || inputSize <= 0 || depth <= 0) {
      return NULL;
  }
    
  float *output = malloc(sizeof(float) * inputSize * inputSize * depth);

  int flatten_idx = 0;
  for (int d = 0; d < depth; d++) {
    for (int i = 0; i < inputSize; i++) {
        for (int j = 0; j < inputSize; j++) {
            output[flatten_idx] = input[d][i][j];
            flatten_idx++; 
        }
    }
  }
  return output;
}

void destroyConvOutput(float ***convOutput, int convOutputSize)
{
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < convOutputSize; j++)
        {
            free(convOutput[i][j]);
        }
        free(convOutput[i]);
    }
    free(convOutput);
}

int forwardPass(float ***image, int numChannels, float ****conv1WeightsData, float **fc1WeightsData, float **fc2WeightsData, float *conv1BiasData, float *fc1BiasData, float *fc2BiasData)
{
    // 1. Perform the convolution operation

    // 2. Flatten the output

    // 3. Perform the fully connected operations

    // 4. Apply the final softmax activation

    // 5. Make predictions

    // Clean up the memory usage
}

int predict(float *probabilityVector, int numClasses)
{
    // TODO: Implement the prediction function
    int predicted = 0;
    for (int c = 1; c < numClasses; c++) {
        if (probabilityVector[c] > probabilityVector[predicted]) {
            predicted = c;
        }
    }
    return predicted;
}
