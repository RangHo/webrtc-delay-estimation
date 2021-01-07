#include "test_tools.h"

#include <cassert>
#include <memory>
#include <random>

void RandomizeSampleVector(rtc::ArrayView<float> samples) {
  constexpr float amplitude = 32767.0f;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(-amplitude, amplitude);

  for (auto& sample : samples)
    sample = dist(gen);
}

void DelayBuffer::Delay(rtc::ArrayView<const float> x, rtc::ArrayView<float> x_delayed) {
  assert(x.size() == x_delayed.size() && "The original buffer and the delayed buffer must have the same size.");
  
  if (this->buffer.empty()) {
    std::copy(x.begin(), x.end(), x_delayed.begin());
  } else {
    for (size_t i = 0; i < x.size(); i++) {
      x_delayed[i] = this->buffer[this->next_insert_index];
      this->buffer[this->next_insert_index] = x[i];
      this->next_insert_index++;
      this->next_insert_index %= this->buffer.size();
    }
  }
}

