#ifndef FSKUBE_H
#define FSKUBE_H

#include "receiversender.h"

namespace fskube {

struct FskParams {
    unsigned int samplesPerSecond;
    unsigned int bitsPerSecond;
    unsigned int markFrequency;
    unsigned int spaceFrequency;
    // Note that this will probably involve truncation, which means
    // that we'll slowly drift. This is fine, as real hardware will
    // drift as well, and we need to be able to deal with that.
    unsigned int samplesPerBit() {
        return samplesPerSecond / bitsPerSecond;
    }
    double secondsPerBit() {
        return 1.0 / bitsPerSecond;
    }
    double samplesToTime(unsigned int samples) {
        return (double) samples / samplesPerSecond;
    }
    bool frequencyToBit(unsigned int frequency) {
        return frequency == markFrequency ? 1 : 0;
    }
};

class Modulator : public Sender<bool, double> {
    private:
        FskParams fsk;
        double continuousPhaseOffset;
    public:
        Modulator(FskParams fsk);
        virtual void receive(bool bit);
};

struct Sample {
    unsigned long long index;
    double value;
    bool valid;
};

class Demodulator : public Sender<double, bool> {
    private:
        FskParams fsk;
        // Histeresis to avoid issues if the signal wavers around zero.
        // See nexus5helloworld for an example of some noise around zero at the
        // start of transmission.
        double highThreshold = 0.4;
        double lowThreshold = -0.4;

        unsigned long long sampleIndex;
        unsigned long long lastZeroCrossingIndex;
        unsigned int insignificantSampleCount;
        Sample lastSignificantSample;

        unsigned int lastFrequencyHalfSeen;
        unsigned int lastFrequencyHalfSeenCount;

        void reset();
        void addZeroCrossing(unsigned long long index);
        void addFrequencyHalfSeen(unsigned int frequency);
    public:
        Demodulator(FskParams fsk);
        virtual void receive(double value);
        void flush();
};

} // namespace fskube

#endif // FSKUBE_H
