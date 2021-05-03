/*
  ==============================================================================

   BodyView

  ==============================================================================
*/

#pragma once

#include "../DspCommon.h"
#include "look_and_feels/IntParamLookAndFeel.h"
#include "look_and_feels/KnobLookAndFeel.h"
#include "look_and_feels/SliderLookAndFeel.h"
#include "look_and_feels/ToggleSwitchLookAndFeel.h"
#include <JuceHeader.h>
#include <array>
#include <utility>

namespace onsen
{
//==============================================================================
class BodyView : public reactjuce::View
{
    struct Position
    {
        int x;
        int y;
    };

    //==============================================================================
public:
    BodyView (juce::AudioProcessorValueTreeState& params)
    {
        for (int i = 0; i < numSlider; i++)
        {
            auto& slider = sliderArray[i];
            slider.setLookAndFeel (&sliderLookAndFeel);
            slider.setSliderStyle (juce::Slider::LinearVertical);
            slider.setMouseDragSensitivity (700);
            addAndMakeVisible (slider);
            sliderAttachmentArray[i].reset (new SliderAttachment (params, sliderNameArray[i], slider));
        }

        // TODO: implement "chorusAdd" parameter
        dummyChorusAddSlider.setLookAndFeel (&sliderLookAndFeel);
        dummyChorusAddSlider.setSliderStyle (juce::Slider::LinearVertical);
        dummyChorusAddSlider.setMouseDragSensitivity (700);
        addAndMakeVisible (dummyChorusAddSlider);

        for (int i = 0; i < numKnob; i++)
        {
            auto& knob = knobArray[i];
            knob.setLookAndFeel (&knobLookAndFeel);
            knob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
            knob.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
            addAndMakeVisible (knob);
            knobAttachmentArray[i].reset (new SliderAttachment (params, knobNameArray[i], knob));
        }

        for (int i = 0; i < numToggleSwitch; i++)
        {
            auto& toggleSwitch = toggleSwitchArray[i];
            toggleSwitch.setLookAndFeel (&toggleSwitchLookAndFeel);
            addAndMakeVisible (toggleSwitch);
            toggleSwitchAttachmentArray[i].reset (new ButtonAttachment (params, toggleSwitchNameArray[i], toggleSwitch));
        }

        for (int i = 0; i < numIntParam; i++)
        {
            auto& intParam = intParamArray[i];
            intParam.setLookAndFeel (&intParamLookAndFeel);
            intParam.setSliderStyle (juce::Slider::RotaryVerticalDrag);
            addAndMakeVisible (intParam);
            intParamAttachmentArray[i].reset (new SliderAttachment (params, intParamNameArray[i], intParam));
        }
    }

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    //==============================================================================
    SliderLookAndFeel sliderLookAndFeel;
    static constexpr int numSlider = 17;

    std::array<juce::Slider, numSlider> sliderArray;
    std::array<std::unique_ptr<SliderAttachment>, numSlider> sliderAttachmentArray;
    std::array<juce::String, numSlider> sliderNameArray {
        "sinGain",
        "squareGain",
        "sawGain",
        "subSquareGain",
        "noiseGain",
        "shape",
        "attack",
        "decay",
        "sustain",
        "release",
        "lfoPhase",
        "lfoDelay",
        "rate",
        "frequency",
        "resonance",
        "portamento",
        "masterVolume"
    };
    std::array<Position, numSlider> sliderPositionArray {
        { { 75, 80 },
          { 115, 80 },
          { 155, 80 },
          { 195, 80 },
          { 235, 80 },
          { 275, 80 },
          { 455, 80 },
          { 495, 80 },
          { 535, 80 },
          { 575, 80 },
          { 75, 355 },
          { 115, 355 },
          { 155, 355 },
          { 355, 355 },
          { 395, 355 },
          { 595, 355 },
          { 775, 355 } }
    };
    // TODO: remove
    static constexpr int sliderXPositionAdjustment = 10;
    static constexpr int sliderYPositionAdjustment = 5;

    // TODO: implement "chorusAdd" parameter
    juce::Slider dummyChorusAddSlider;

    //==============================================================================
    KnobLookAndFeel knobLookAndFeel;
    static constexpr int numKnob = 4;

    std::array<juce::Slider, numKnob> knobArray;
    std::array<std::unique_ptr<SliderAttachment>, numKnob> knobAttachmentArray;
    std::array<juce::String, numKnob> knobNameArray {
        "lfoShape",
        "filterEnv",
        "lfoFilterFreq",
        "lfoPitch"
    };
    std::array<Position, numKnob> knobPositionArray {
        { { 310, 140 },
          { 430, 360 },
          { 430, 420 },
          { 630, 420 } }
    };

    //==============================================================================
    ToggleSwitchLookAndFeel toggleSwitchLookAndFeel;
    static constexpr int numToggleSwitch = 3;

    std::array<juce::ToggleButton, numToggleSwitch> toggleSwitchArray;
    std::array<std::unique_ptr<ButtonAttachment>, numToggleSwitch> toggleSwitchAttachmentArray;
    std::array<juce::String, numToggleSwitch> toggleSwitchNameArray {
        "envForAmpOn",
        "chorusOn", // TODO: Change to "chorusType"
        "syncOn"
    };
    std::array<Position, numToggleSwitch> toggleSwitchPositionArray {
        { { 617, 97 },
          { 797, 97 },
          { 197, 438 } }
    };

    //==============================================================================
    IntParamLookAndFeel intParamLookAndFeel;
    static constexpr int numIntParam = 6;

    std::array<juce::Slider, numIntParam> intParamArray;
    std::array<std::unique_ptr<SliderAttachment>, numIntParam> intParamAttachmentArray;
    std::array<juce::String, numIntParam> intParamNameArray {
        "numVoices",
        "rateSync",
        "pitchBendWidth",
        "masterOctaveTune",
        "masterSemitoneTune",
        "masterFineTune"

    };
    std::array<Position, numIntParam> intParamPositionArray {
        {
            { 319, 90 },
            { 199, 371 },
            { 640, 371 },
            { 700, 372 },
            { 700, 430 },
            { 700, 490 },
        }
    };
};
} // namespace onsen
