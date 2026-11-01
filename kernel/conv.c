#include "conv.h"

// Im2col algorithm
float **im2col(float ***image, int numChannels, int imageSize, int kernelSize, int stride, int *outputSize)
{
    // TODO: Implement the im2col algorithm
    int outputSize = (imageSize - kernelSize)/stride + 1;

    

}

// Im2col algorithm's inverse
float ***col2im(float **result, int num_kernels, int conv_rows, int conv_cols)
{
    // TODO: Implement the col2im algorithm
}

float **kernel_flatten(float ****kernel, int num_kernels, int kernel_size)
{
    float **flattened_kernels = (float **)malloc(num_kernels * sizeof(float *));
    for (int i = 0; i < num_kernels; i++)
    {
        flattened_kernels[i] = (float *)malloc(kernel_size * kernel_size * sizeof(float));
    }

    for (int k = 0; k < num_kernels; k++)
    {
        for (int i = 0; i < kernel_size; i++)
        {
            for (int j = 0; j < kernel_size; j++)
            {
                flattened_kernels[k][i * kernel_size + j] = kernel[k][0][i][j];
            }
        }
    }

    return flattened_kernels;
}

// Basic convolution operation
float ***convolution(float ***image, int numChannels, float ****kernel, float *biasData, int numFilters, int inputSize, int kernelSize)
{
    // TODO: Implement the basic convolution operation
    // outputSize = (inputSize - kernelSize)/stride + 1
    int outputSize = inputSize - kernelSize + 1;

    float ***output = malloc(sizeof(float **) * numFilters);

    for (int f = 0; f < numFilters; f++){
        output[f] = malloc(sizeof(float *) * outputSize);

        for (int i = 0; i < outputSize; i++){
            output[f][i] = malloc(sizeof(float) * outputSize);

            for (int j = 0; j < outputSize; j++){
                output[f][i][j] = 0.00f;

                for (int c = 0; c < numChannels; c++) {
                    for (int m = 0; m < kernelSize; m++) {
                        for (int n = 0; n < kernelSize; n++){
                            output[f][i][j] += image[c][i+m][j+n] * kernel[f][c][m][n];
                        }
                    }
                }
                output[f][i][j] += biasData[f];
                output[f][i][j] = relu(output[f][i][j]);
            }
        }
    }

    return output;
}

// Convolution with im2col algorithm
float ***convolution_im2col(float ***image, int numChannels, float ****kernel, float *biasData, int numFilters, int inputSize, int kernelSize, MatmulType matmul_type)
{
    // TODO: Implement the convolution operation with im2col algorithm
    // Flatten kernel

    // Apply im2col

    // Apply matmul

    // Apply col2im

    // Add bias and apply ReLU

    // Cleanup
}