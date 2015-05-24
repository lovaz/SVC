#ifndef SAMPLE_HPP
#define SAMPLE_HPP

#include <string.h>


#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (128)
#define NUM_CHANNELS    (1)
#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SIZE (1)
#define SAMPLE_SILENCE  (0.0f)



class Sample
{
private:
	float sample[FRAMES_PER_BUFFER];
public:
    void setSample(float* data)
    {
    	std::copy(data, data+128, sample);
    }
    float* getSample()
    {
        return sample;
    }
};

#endif