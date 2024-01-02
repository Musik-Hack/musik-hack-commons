/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_dsp/juce_dsp.h>

//==============================================================================
LockfreeExampleProcessor::LockfreeExampleProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      random(234), vizQueue(2048), meterQueue(128)
#endif
{
}

LockfreeExampleProcessor::~LockfreeExampleProcessor() {}

//==============================================================================
const juce::String LockfreeExampleProcessor::getName() const {
  return JucePlugin_Name;
}

bool LockfreeExampleProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool LockfreeExampleProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool LockfreeExampleProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double LockfreeExampleProcessor::getTailLengthSeconds() const { return 0.0; }

int LockfreeExampleProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int LockfreeExampleProcessor::getCurrentProgram() { return 0; }

void LockfreeExampleProcessor::setCurrentProgram(int /*index*/) {}

const juce::String LockfreeExampleProcessor::getProgramName(int /*index*/) {
  return {};
}

void LockfreeExampleProcessor::changeProgramName(
    int /*index*/, const juce::String & /*newName*/) {}

//==============================================================================
void LockfreeExampleProcessor::prepareToPlay(double sr,
                                             int /*samplesPerBlock*/) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  const auto samplesPerCycle = sr / 120.;
  step =
      static_cast<float>(juce::MathConstants<double>::twoPi / samplesPerCycle);
  vizQueue.resize(static_cast<int>(sr / 60));
}

void LockfreeExampleProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LockfreeExampleProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void LockfreeExampleProcessor::processBlock(
    juce::AudioBuffer<float> &buffer, juce::MidiBuffer & /*midiMessages*/) {
  juce::ScopedNoDenormals noDenormals;

  // don't care, just doing mono
  const auto slowStep = step * 0.001f;
  auto block = juce::dsp::AudioBlock<float>(buffer);
  auto data = block.getChannelPointer(0);
  const auto numSamples = block.getNumSamples();
  for (size_t s = 0; s < numSamples; s++) {
    const auto sample = std::sin(phase) * std::sin(slowPhase);
    phase += step;
    slowPhase += slowStep;
    data[s] = sample;
    vizQueue.push(sample);
  }

  if (phase > juce::MathConstants<float>::twoPi) {
    phase -= juce::MathConstants<float>::twoPi;
  }

  if (slowPhase > juce::MathConstants<float>::twoPi) {
    slowPhase -= juce::MathConstants<float>::twoPi;
  }

  const auto minMax = block.findMinAndMax();
  const auto max =
      std::max(std::abs(minMax.getStart()), std::abs(minMax.getEnd()));
  meterQueue.push({MeterType::peak, max});

  if (numSamples > 0) {
    meterQueue.push({MeterType::random, random.nextFloat()});
  }
}

//==============================================================================
bool LockfreeExampleProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *LockfreeExampleProcessor::createEditor() {
  return new LockfreeExampleEditor(*this);
}

//==============================================================================
void LockfreeExampleProcessor::getStateInformation(
    juce::MemoryBlock & /*destData*/) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void LockfreeExampleProcessor::setStateInformation(const void * /*data*/,
                                                   int /*sizeInBytes*/) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new LockfreeExampleProcessor();
}
