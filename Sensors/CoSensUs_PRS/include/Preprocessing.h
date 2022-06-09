static float prReturnArray[2];

#if !FFTON
#if NNON
static float tmp_x_test[N_INPUTS];
#else
static float tmp_x_test[SAMPLES];
float *X_test_ptr;
float X_test[SAMPLES];
#endif

int sort_asc(const void *cmp1, const void *cmp2)
{
    float a = *((float *)cmp1);
    float b = *((float *)cmp2);
    // The comparison
    return a < b ? -1 : (a > b ? 1 : 0);
}

float *preProcessInputs()
{
    // Calculate mean and std
    for (int i = 0; i < BUFFER_NUM; i++)
    {
        int16_t temp_array[SAMPLES];
        float temp_sum = 0, temp_dev_sum = 0, temp_std = 0, temp_mean = 0, temp_median = 0, temp_25 = 0, temp_75 = 0;

        // Calculate mean
        for (int j = 0; j < SAMPLES; j++)
        {
            temp_array[j] = float(buffers[i]->shift());
            temp_sum += temp_array[j];
        }

        temp_mean = temp_sum / float(SAMPLES);

        // Sort
        int array_length = sizeof(temp_array) / sizeof(temp_array[0]);
        qsort(temp_array, array_length, sizeof(temp_array[0]), sort_asc);

        // Mediam and Percentiles (and completely statistically correct, but good enough for our purposes)
        temp_median = temp_array[int(SAMPLES * 2 / 4)];
        temp_25 = temp_array[int(SAMPLES * 1 / 4)];
        temp_75 = temp_array[int(SAMPLES * 3 / 4)];

        // Calculate std
        for (int j = 0; j < SAMPLES; j++)
        {
            temp_dev_sum += pow((temp_mean - temp_array[j]), 2);
        }

        temp_std = sqrt(temp_dev_sum / float(SAMPLES));

        // Write values to X
        tmp_x_test[i * 5 + 0] = temp_mean;
        tmp_x_test[i * 5 + 1] = temp_std;
        tmp_x_test[i * 5 + 2] = temp_25;
        tmp_x_test[i * 5 + 3] = temp_median;
        tmp_x_test[i * 5 + 4] = temp_75;
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