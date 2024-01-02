/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
 */
class LockfreeExampleEditor : public juce::AudioProcessorEditor,
                              public juce::Timer {
public:
  LockfreeExampleEditor(LockfreeExampleProcessor &);
  ~LockfreeExampleEditor() override;
  void timerCallback() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  LockfreeExampleProcessor &audioProcessor;
  juce::AudioFormatManager formatManager;
  juce::Array<juce::File> sampleFiles;
  juce::Slider fileSelector;
  juce::Label title;
  int xPos = 0;
  float lastMeterVal = 0.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockfreeExampleEditor)
};
