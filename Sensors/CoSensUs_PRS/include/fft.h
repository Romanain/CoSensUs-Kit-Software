#include <esp_dsp.h>
#define BINS 8

float wind[SAMPLES];
static float x1[SAMPLES];

void initializeFFT()
{
    esp_err_t ret;
    ESP_LOGI(TAG, "Start Example.");
    ret = dsps_fft4r_init_fc32(NULL, SAMPLES >> 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Not possible to initialize FFT4R. Error = %i", ret);
        return;
    }
}

float *runFFT(CircularBuffer<float, SAMPLES + SAMPLE_SURPLUS> *sampleBuffer)
{
    // Generate hann window
    dsps_wind_hann_f32(wind, SAMPLES);

    // Convert two input vectors to one complex vector
    for (int i = 0; i < SAMPLES; i++)
    {
        x1[i] = sampleBuffer->shift() * wind[i];
    }

    dsps_fft4r_fc32(x1, SAMPLES >> 1);
    // Bit reverse
    dsps_bit_rev4r_fc32(x1, SAMPLES >> 1);
    // Convert one complex vector with length N/2 to one real spectrum vector with length N/2
    dsps_cplx2real_fc32(x1, SAMPLES >> 1);

    for (int i = 0; i < SAMPLES / 2; i++)
    {
        x1[i] = 10 * log10f((x1[i * 2 + 0] * x1[i * 2 + 0] + x1[i * 2 + 1] * x1[i * 2 + 1] + 0.0000001) / SAMPLES);
    }

    return x1;
}

static float binReturnArray[BINS];

float *FFTBinner(float *FFTArray)
{
    int binSize = SAMPLES / 2 / BINS;

    for (int b = 0; b < BINS; b++)
    {
        float tmp = 0;
        for (int i = 0; i < binSize; i++)
        {
            tmp += FFTArray[b + i];
        }
        binReturnArray[b] = tmp;
    }

    return binReturnArray;
}


