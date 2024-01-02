/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <musikhack/lockfree/lockfree.h>

//==============================================================================
/**
 */

enum class MeterType { peak, rms };
using MeterPair = std::pair<MeterType, float>;

class LockfreeExampleProcessor : public juce::AudioProcessor {
public:
  //==============================================================================
  LockfreeExampleProcessor();
  ~LockfreeExampleProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  musikhack::lockfree::AsyncFifo<float> &getVizQueue() { return vizQueue; }
  musikhack::lockfree::AsyncFifo<MeterPair> &getMeterQueue() {
    return meterQueue;
  }

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  // call via the editor/message thread
  void queueSoundLoad(musikhack::lockfree::LoadableSound::Options const &opts) {
    soundLoader.load(opts);
  }

private:
  size_t samplePosition = 0;

  juce::AudioBuffer<float> RMSBuffer;
  size_t RMSBufferPosition = 0;

  musikhack::lockfree::AsyncFifo<float> vizQueue;
  musikhack::lockfree::AsyncFifo<MeterPair> meterQueue;
  musikhack::lockfree::SoundLoader soundLoader;
  std::unique_ptr<musikhack::lockfree::LoadableSound> loadedSound;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockfreeExampleProcessor)
};
