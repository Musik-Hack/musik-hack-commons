#pragma once

#if 0

     BEGIN_JUCE_MODULE_DECLARATION

      ID:               lockfree
      vendor:           Musik Hack LLC
      version:          1.0.0
      name:             lockfree
      description:      helper functions for moodycamel single writer, single consumer lock-free queues
      license:          Apache 2
      dependencies:     juce_core

     END_JUCE_MODULE_DECLARATION

#endif

#include "deps/readerwriterqueue/readerwriterqueue.h"
#include <functional>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

namespace musikhack {
namespace lockfree {

template <typename T> class AsyncFifo {
public:
  using CallBack = std::function<void(T &)>;
  using TypedQueue = moodycamel::ReaderWriterQueue<T>;

  AsyncFifo(int size = 1024) : queue(size) {}

  // Resize the queue, but deletes all existing data.
  // Not thread safe!
  void resize(int size) { queue = TypedQueue(size); }

  // Wipes out the queue and resets it to its initial state
  // Not thread safe!
  void clear() { queue = TypedQueue(queue.max_capacity()); }

  // push an item into the queue
  bool push(T const &item) { return queue.try_enqueue(item); }

  // push an item into the queue using move semantics
  bool push(T &&item) { return queue.try_enqueue(item); }

  // push an item into the queue using emplace semantics
  template <typename... Args> bool emplace(Args &&...args) {
    return queue.try_emplace(std::forward<Args>(args)...);
  }

  // pop from the queue into item
  bool pop(T &item) { return queue.try_dequeue(item); }

  // pop each item out of the queue and run a callback on it
  void forEach(CallBack cbk) {
    T item;
    while (pop(item))
      cbk(item);
  }

  // pop each item out of the queue, but only run the callback on the last one
  void forLast(CallBack cbk) {
    bool ran = false;
    T item;
    while (pop(item)) {
      ran = true;
    }

    if (ran)
      cbk(item);
  }

  // return a reference to the underlying queue
  TypedQueue &getQueue() const noexcept { return queue; }

private:
  TypedQueue queue;
};

template <typename T, typename Options> class Loader : public juce::Thread {
  static_assert(std::is_constructible_v<T, Options>,
                "T must be constructible with Options");
  static_assert(std::is_default_constructible_v<Options>,
                "Options must be default constructible");

public:
  // A unique pointer to the object type
  using ObjPtr = std::unique_ptr<T>;

  // A single reader single writer queue for object creation.
  // The options for initializing the object are their own type
  // and the object's initializer should take that type as an argument
  using OptionsQueue = moodycamel::ReaderWriterQueue<Options>;

  // A single reader single writer queue for the objects themselves
  // used to pass the objects back to the audio thread
  using ObjectQueue = moodycamel::ReaderWriterQueue<ObjPtr>;

  // A callback for when an object is grabbed from the queue by the
  // audio thread. The object is passed as an ObjPtr
  using CallBack = std::function<void(ObjPtr)>;

  Loader(const juce::String &name, size_t initialSize = 25,
         bool shouldOnlyUseLastMessage = false)
      : juce::Thread(name), onlyUseLastMessage(shouldOnlyUseLastMessage),
        toLoad(initialSize), loaded(initialSize), toDestroy(initialSize) {}

  // Load options into the queue for creation
  bool load(Options &&creator) {
    const auto ret = toLoad.try_enqueue(creator);
    notify();
    return ret;
  }

  // Load options into the queue for creation
  bool load(const Options &creator) {
    const auto ret = toLoad.try_enqueue(creator);
    notify();
    return ret;
  }

  // Queue an object for destruction
  void destroy(ObjPtr object) {
    toDestroy.enqueue(std::move(object));
    notify();
  }

  // Pop a single loaded object from the queue
  bool getLoaded(ObjPtr &loadedT) { return loaded.try_dequeue(loadedT); }

  // For each loaded object, run a callback on that object
  void forEach(CallBack cbk) {
    ObjPtr t;
    while (loaded.try_dequeue(t))
      cbk(std::move(t));
  }

  // Start the loader background thread
  void run() override {
    while (true) {
      if (threadShouldExit())
        return;

      loadAndDestroy();

      if (threadShouldExit())
        return;

      wait(-1);
    }
  }

  ~Loader() override {}

private:
  void loadAndDestroy() {
    Options creator;

    if (onlyUseLastMessage) {
      auto atLeastOne = false;
      while (toLoad.try_dequeue(creator)) {
        atLeastOne = true;
      }
      if (atLeastOne) {
        loaded.try_enqueue(std::make_unique<T>(creator));
      }
    } else {
      while (toLoad.try_dequeue(creator)) {
        loaded.try_enqueue(std::make_unique<T>(creator));
      }
    }

    // Destroy objects that are no longer used
    ObjPtr object;
    while (toDestroy.try_dequeue(object)) {
      object.reset();
    }
  }

  // Whenever looping through the options to load into objects, only use
  // the last one in the queue
  bool onlyUseLastMessage;

  // Some other thread (the GUI or audio thread, but not both!) writes creator
  // structs, Loader consumes them in its own thread
  OptionsQueue toLoad;

  // Loader pushes objects to this queue in its own thread after creating them,
  // and the audio thread consumes them
  ObjectQueue loaded;

  // The audio thread pushes objects to this queue, Loader destroys them in its
  // own thread
  ObjectQueue toDestroy;
};

class LoadableSound {

public:
  struct Options {
    juce::String name;
    juce::File path;
    juce::AudioFormatManager *formatManager;
  };

  LoadableSound(Options const &opts) : name(opts.name) {
    // Bail if the file doesn't exist
    if (!opts.path.existsAsFile()) {
      return;
    }

    auto reader = std::unique_ptr<juce::AudioFormatReader>(
        opts.formatManager->createReaderFor(opts.path));

    if (!reader) {
      return;
    }

    const int numChannels = (int)reader->numChannels;
    const int numSamples = (int)reader->lengthInSamples;

    buffer.setSize(numChannels, numSamples);
    auto writePointers = buffer.getArrayOfWritePointers();

    if (reader->read(writePointers, numChannels, 0, numSamples)) {
      valid = true;
    };
  }

  juce::String getName() const { return name; }

  juce::dsp::AudioBlock<float> getBlock(size_t startSample, size_t numSamples) {
    const auto numSamplesInBuffer = getNumSamples();

    if (numSamplesInBuffer == 0) {
      return juce::dsp::AudioBlock<float>();
    }

    startSample = juce::jmin(startSample, numSamplesInBuffer - 1);
    numSamples = juce::jmin(numSamples, numSamplesInBuffer - startSample);
    return juce::dsp::AudioBlock<float>(buffer).getSubBlock(startSample,
                                                            numSamples);
  }

  size_t getNumChannels() const { return (size_t)buffer.getNumChannels(); }
  size_t getNumSamples() const { return (size_t)buffer.getNumSamples(); }

private:
  bool valid = false;
  juce::String name;
  juce::AudioBuffer<float> buffer;
};

using SoundLoader = Loader<LoadableSound, LoadableSound::Options>;

} // namespace lockfree
} // namespace musikhack
