#include "conv.h"
#include <string.h>

// Im2col algorithm
float **im2col(float ***image, int numChannels, int imageSize, int kernelSize, int stride, int *outputSize)
{
    // TODO: Implement the im2col algorithm
    const int outputSide = (imageSize - kernelSize) / stride + 1;
    const size_t numRows = numChannels * kernelSize * kernelSize;
    const size_t numCols = outputSide * outputSide;
    float **columns = malloc(numRows * sizeof(*columns));
    if (columns == NULL)
        return NULL;

    size_t row = 0;
    for (int c = 0; c < numChannels; c++)
    {
        float **channel = image[c];
        for (int ky = 0; ky < kernelSize; ky++)
        {
            for (int kx = 0; kx < kernelSize; kx++, row++)
            {
                columns[row] = malloc(numCols * sizeof(*columns[row]));
                if (columns[row] == NULL)
                {
                    while (row > 0)
                        free(columns[--row]);
                    free(columns);
                    return NULL;
                }

                float *dst = columns[row];
                for (int y = 0; y < outputSide; y++)
                {
                    const float *src = channel[ky + y * stride] + kx;
                    for (int x = 0; x < outputSide; x++)
                        *dst++ = src[x * stride];
                }
            }
        }
    }
    *outputSize = outputSide;
    return columns;
}

// Im2col algorithm's inverse
float ***col2im(float **result, int num_kernels, int conv_rows, int conv_cols)
{
    // TODO: Implement the col2im algorithm
    float ***image = malloc(num_kernels * sizeof(*image));
    const size_t row_bytes = conv_cols * sizeof(float);
    int k = 0;
    int row = conv_rows;
    while (image != NULL && k < num_kernels && row == conv_rows &&
           (image[k] = malloc(conv_rows * sizeof(*image[k]))) != NULL)
    {
        const float *src = result[k];
        for (row = 0; row < conv_rows &&
                      (image[k][row] = malloc(row_bytes)) != NULL; row++)
        {
            memcpy(image[k][row], src, row_bytes);
            src += conv_cols;
        }
        if (row == conv_rows)
            k++;
        else
        {
            for (int i = 0; i < row; i++)
                free(image[k][i]);
            free(image[k]);
        }
    }

    if (image != NULL && k < num_kernels)
    {
        while (k > 0)
        {
            --k;
            for (int row = 0; row < conv_rows; row++)
                free(image[k][row]);
            free(image[k]);
        }
        free(image);
        image = NULL;
    }
    return image;
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

    const int kernelElements = numChannels * kernelSize * kernelSize;
    float **weights = malloc(numFilters * sizeof(*weights));
    float *weightData = malloc(numFilters * kernelElements * sizeof(*weightData));
    if (weights == NULL || weightData == NULL)
    {
        free(weights);
        free(weightData);
        return NULL;
    }

    for (int f = 0; f < numFilters; f++)
    {
        float *dst = weightData + f * kernelElements;
        weights[f] = dst;
        for (int c = 0; c < numChannels; c++)
            for (int y = 0; y < kernelSize; y++)
            {
                memcpy(dst, kernel[f][c][y], kernelSize * sizeof(*dst));
                dst += kernelSize;
            }
    }

    int outputSize;
    float **columns = im2col(image, numChannels, inputSize, kernelSize, 1, &outputSize);
    if (columns == NULL)
    {
        free(weightData);
        free(weights);
        return NULL;
    }

    const int outputElements = outputSize * outputSize;
    float **result = matmul(weights, columns, numFilters, kernelElements, kernelElements, outputElements);

    free(weightData);
    free(weights);
    for (int i = 0; i < kernelElements; i++)
        free(columns[i]);
    free(columns);
    if (result == NULL)
        return NULL;

    for (int f = 0; f < numFilters; f++)
    {
        float *row = result[f];
        const float bias = biasData[f];
        for (int i = 0; i < outputElements; i++)
            row[i] = relu(row[i] + bias);
    }

    float ***output = col2im(result, numFilters, outputSize, outputSize);
    for (int f = 0; f < numFilters; f++)
        free(result[f]);
    free(result);
    return output;
}