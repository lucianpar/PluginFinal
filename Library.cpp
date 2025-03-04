#include "Library.h"

namespace ky {

// The one and only way to get an instance of PlaybackRateSubject
PlaybackRateSubject& PlaybackRateSubject::instance() {
  static PlaybackRateSubject instance;
  return instance;
}

// the default action for PlaybackRateObserver
void PlaybackRateObserver::onPlaybackRateChange(float rate) {
  samplerate = rate;
}

// When a PlaybackRateObserver is born, it adds itself to the list of observers
PlaybackRateObserver::PlaybackRateObserver() {
  PlaybackRateSubject::instance().addObserver(this);
}

void PlaybackRateSubject::notifyObservers(float samplerate) {
  for (auto* o = list; o != nullptr; o = o->nextObserver) {
    o->onPlaybackRateChange(samplerate);
  }
}

void PlaybackRateSubject::addObserver(PlaybackRateObserver* observer) {
  observer->nextObserver = list;
  list = observer;
}

}  // namespace ky