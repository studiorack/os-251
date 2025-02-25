/*
  ==============================================================================

   Master volume test

  ==============================================================================
*/

#include "../../src/dsp/MasterVolume.h"
#include "../../src/params/MasterParamsMock.h"
#include "util/AudioBufferMock.h"
#include "util/TestAudioBufferInput.h"
#include <gtest/gtest.h>

namespace onsen
{
//==============================================================================
// Master volume

class MasterVolumeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set all value to 1.0
        setTestInput2Constant (&audioBuffer, 1.0);
    }
    // void TearDown() override {}

    static constexpr int samplesPerBlock = 512;
    static constexpr int numChannels = 2;
    // This value depends on MasterVolume implementation.
    static constexpr flnum gainAdjustment = 0.2;
    // This value depends on MasterVolume implementation.
    static constexpr flnum clippingValue = 2.0;
    MasterParamsMock masterParam { false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    AudioBufferMock audioBuffer { numChannels, samplesPerBlock };
    MasterVolume masterVolume { &masterParam };
};

TEST_F (MasterVolumeTest, Volume0db)
{
    masterParam.setMasterVolume (1.0 /*0 [dB]*/);
    masterVolume.render (&audioBuffer, 0, samplesPerBlock);
    for (int i = 0; i < audioBuffer.getNumChannels(); i++)
    {
        for (int j = 0; j < audioBuffer.getNumSamples(); j++)
        {
            EXPECT_FLOAT_EQ (audioBuffer.getSample (i, j), 1.0 * gainAdjustment);
        }
    }
}

TEST_F (MasterVolumeTest, VolumeMinus3db)
{
    masterParam.setMasterVolume (0.5 /*-3.0 [dB]*/);
    masterVolume.render (&audioBuffer, 0, samplesPerBlock);
    for (int i = 0; i < audioBuffer.getNumChannels(); i++)
    {
        for (int j = 0; j < audioBuffer.getNumSamples(); j++)
        {
            EXPECT_FLOAT_EQ (audioBuffer.getSample (i, j), 0.5 * gainAdjustment);
        }
    }
}

TEST_F (MasterVolumeTest, ClipExtremeValue1)
{
    // Set the buffer with extreme value
    setTestInput2Constant (&audioBuffer, 100.0);
    masterParam.setMasterVolume (1.0);

    masterVolume.render (&audioBuffer, 0, samplesPerBlock);
    for (int i = 0; i < audioBuffer.getNumChannels(); i++)
    {
        for (int j = 0; j < audioBuffer.getNumSamples(); j++)
        {
            EXPECT_FLOAT_EQ (audioBuffer.getSample (i, j), clippingValue);
        }
    }
}

TEST_F (MasterVolumeTest, ClipExtremeValue2)
{
    // Set the buffer with extreme value
    setTestInput2Constant (&audioBuffer, -100.0);
    masterParam.setMasterVolume (1.0);

    masterVolume.render (&audioBuffer, 0, samplesPerBlock);
    for (int i = 0; i < audioBuffer.getNumChannels(); i++)
    {
        for (int j = 0; j < audioBuffer.getNumSamples(); j++)
        {
            EXPECT_FLOAT_EQ (audioBuffer.getSample (i, j), -clippingValue);
        }
    }
}

TEST_F (MasterVolumeTest, RenderPartOfBuffer)
{
    masterParam.setMasterVolume (0.5 /*-3.0 [dB]*/);
    const int renderStart = samplesPerBlock / 10;
    const int numSamplesToRender = samplesPerBlock / 5;
    masterVolume.render (&audioBuffer, renderStart, numSamplesToRender);
    for (int i = 0; i < audioBuffer.getNumChannels(); i++)
    {
        for (int j = 0; j < audioBuffer.getNumSamples(); j++)
        {
            if (j >= renderStart && j <= renderStart + numSamplesToRender - 1)
            {
                // Should have new value
                EXPECT_FLOAT_EQ (audioBuffer.getSample (i, j), 0.5 * gainAdjustment);
            }
            else
            {
                // Should have original value
                EXPECT_FLOAT_EQ (audioBuffer.getSample (i, j), 1.0);
            }
        }
    }
}
} // namespace onsen
