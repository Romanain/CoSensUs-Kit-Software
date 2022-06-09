#include "MVP_Sensor_PR_tflite_model.h"

// VARIABLES TINYML
#define N_INPUTS 12
#define N_OUTPUTS 3
#define TENSOR_ARENA_SIZE 16 * 1024

float y_pred = 0;
float *X_test_ptr;
float X_test[N_INPUTS];

Eloquent::TinyML::TensorFlow::TensorFlow<N_INPUTS, N_OUTPUTS, TENSOR_ARENA_SIZE> tf;

void StartNN()
{
    tf.begin(model_data);
    // check if model loaded fine
    if (!tf.isOk())
    {
#if DEBUG
        Serial << "ERROR: " << tf.getErrorMessage();
#endif
        while (true)
            delay(1000);
    }
}

float PredictNN()
{
    // Predict outcome and time it
    uint32_t start = micros();
    y_pred = tf.predictClass(X_test);
    uint32_t timeit = micros() - start;

#if DEBUG
    Serial << "\nPrediction: " << y_pred << " (" << timeit << "us)\n";
#endif

    return y_pred;
}