static float prReturnArray[2];

#if !FFTON
#if NNON
static float tmp_x_test[N_INPUTS];
#else
static float tmp_x_test[SAMPLES];
float *X_test_ptr;
float X_test[SAMPLES];
#endif

float *preProcessInputs()
{
    // Calculate mean and std
    for (int i = 0; i < BUFFER_NUM; i++)
    {
        int16_t temp_array[SAMPLES];
        float temp_sum = 0, temp_dev_sum = 0, temp_std = 0, temp_mean = 0;

        // Calculate mean
        for (int j = 0; j < SAMPLES; j++)
        {
            temp_array[j] = float(buffers[i]->shift());
            temp_sum += temp_array[j];
        }

        temp_mean = temp_sum / float(SAMPLES);

        // Calculate std
        for (int j = 0; j < SAMPLES; j++)
        {
            temp_dev_sum += pow((temp_mean - temp_array[j]), 2);
        }

        temp_std = sqrt(temp_dev_sum / float(SAMPLES));

        // Write values to X
        tmp_x_test[i * 2] = temp_mean;
        tmp_x_test[i * 2 + 1] = temp_std;
    }

    return tmp_x_test;
}
#endif

#if FFTON
float *preProcessInputs(float *FFTBins)
{
    // Calculate mean and std

    int16_t temp_array[BINS];
    float temp_sum = 0, temp_dev_sum = 0, temp_std = 0, temp_mean = 0;

    // Calculate mean
    for (int j = 0; j < BINS; j++)
    {
        temp_array[j] = FFTBins[j];
        temp_sum += temp_array[j];
    }

    temp_mean = temp_sum / float(BINS);

    // Calculate std
    for (int j = 0; j < BINS; j++)
    {
        temp_dev_sum += pow((temp_mean - temp_array[j]), 2);
    }

    temp_std = sqrt(temp_dev_sum / float(BINS));

    // Write values to X
    prReturnArray[0] = temp_mean;
    prReturnArray[1] = temp_std;

    return prReturnArray;
}
#endif