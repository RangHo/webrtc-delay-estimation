#ifndef TEST_TOOLS_H_
#define TEST_TOOLS_H_

#include <cstddef>
#include <vector>

#include "array_view.h"

void RandomizeSampleVector(rtc::ArrayView<float> sample);

class DelayBuffer {
  public:
    explicit DelayBuffer(size_t delay): buffer(delay) {}
    ~DelayBuffer() = default;

    void Delay(rtc::ArrayView<const float> x, rtc::ArrayView<float> x_delayed);

  private:
    std::vector<float> buffer;
    size_t next_insert_index = 0;
};

#endif

