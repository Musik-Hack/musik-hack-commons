/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <musikhack/lockfree/lockfree.h>
#include <unordered_map>

//==============================================================================
/**
 */

struct Logger {
  enum class ID { UNKNOWN, NEW_SOUND, LOOP, RANDOM_MESSAGE };
  const inline static std::unordered_map<ID, juce::String> TEMPLATES = {
      {ID::NEW_SOUND, "New sound loaded"},
      {ID::LOOP, "Sound looped {0}} times"},
      {ID::UNKNOWN, "UNKNOWN MESSAGE"},
      {ID::RANDOM_MESSAGE, "Random message {0}"}};

  struct Message {
    ID id = ID::UNKNOWN;
    double dbl = 0.;
  };

  static juce::String format(Message m) {
    const auto msg = TEMPLATES.find(m.id);
    if (msg != TEMPLATES.end()) {
      auto str = msg->second;
      str = str.replace("{0}", juce::String(m.dbl));
      return str;
    }
    return TEMPLATES.at(ID::UNKNOWN);
  }
};

class LockfreeExampleProcessor : public juce::AudioProcessor {
public:
  //==============================================================================
  LockfreeExampleProcessor();
  ~LockfreeExampleProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  musikhack::lockfree::Ring<float> &getVizRing() { return vizRing; }

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

  musikhack::lockfree::Queue<Logger::Message> &getLogQueue() {
    return logQueue;
  }

  float getPeak() const { return peakMeter.load(); }
  float getRMS() const { return rmsMeter.load(); }

private:
  size_t samplePosition = 0;
  size_t loopCount = 0;

  juce::AudioBuffer<float> RMSBuffer;
  size_t RMSBufferPosition = 0;

  std::atomic<float> peakMeter = 0.f;
  std::atomic<float> rmsMeter = 0.f;

  musikhack::lockfree::Ring<float> vizRing;
  musikhack::lockfree::Queue<Logger::Message> logQueue;
  musikhack::lockfree::SoundLoader soundLoader;
  std::unique_ptr<musikhack::lockfree::LoadableSound> loadedSound;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockfreeExampleProcessor)
};
