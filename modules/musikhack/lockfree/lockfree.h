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
#include <juce_core/juce_core.h>

namespace musikhack {
namespace lockfree {

template <typename T> class AsyncFifo {
public:
  using CallBack = std::function<void(T &)>;
  using TypedQueue = moodycamel::ReaderWriterQueue<T>;

  AsyncFifo(int size = 1024) : queue(size) {}

  // Resize the queue, but deletes all existing data
  void resize(int size) { queue = TypedQueue(size); }
  void clear() { queue = TypedQueue(queue.max_capacity()); }

  // push an item into the queue
  bool push(T const &t) { return queue.try_enqueue(t); }

  // push an item into the queue using move semantics
  bool push(T &&t) { return queue.try_enqueue(t); }

  // push an item into the queue using emplace semantics
  template <typename... Args> bool emplace(Args &&...args) {
    return queue.try_emplace(std::forward<Args>(args)...);
  }

  // pop an item from the queue
  bool pop(T &t) { return queue.try_dequeue(t); }

  void forEach(CallBack cbk) {
    T t;
    while (pop(t))
      cbk(t);
  }

  TypedQueue &getQueue() const noexcept { return queue; }

private:
  TypedQueue queue;
};

template <typename T, typename Options> class Loader : public juce::Thread {
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
         bool shouldOnlyUseLastMessage = false, int timeoutInMs = 100)
      : juce::Thread(name), onlyUseLastMessage(shouldOnlyUseLastMessage),
        timeout(timeoutInMs), toLoad(initialSize), loaded(initialSize),
        toDestroy(initialSize) {}

  // Load options into the queue for creation
  bool load(Options &&creator) {
    auto enqueued = creator ? toLoad.try_enqueue(creator) : false;
    return enqueued;
  }

  // Queue an object for destruction
  void destroy(ObjPtr object) { toDestroy.enqueue(std::move(object)); }

  // Pop a single loaded object from the queue
  bool getLoaded(ObjPtr &loadedT) { return loaded.try_dequeue(loadedT); }

  // For each loaded object, run a callback on that object
  void forEach(CallBack cbk) {
    T t;
    while (pop(t))
      cbk(t);
  }

  // Start the loader background thread
  void run() override {
    while (true) {
      if (threadShouldExit())
        return;

      loadAndDestroy();

      if (threadShouldExit())
        return;

      sleep(timeout);
    }
  }

  ~Loader() override {}

private:
  void loadAndDestroy() {
    Options creator;

    if (onlyUseLastMessage) {
      while (toLoad.try_dequeue(creator)) {
      }
      if (creator) {
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

  // The timeout interval for the thread to sleep between queue checks
  int timeout;

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
} // namespace lockfree
} // namespace musikhack
