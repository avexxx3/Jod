#include <cmath>
#include <iostream>
#include <portaudio.h>
#include <sndfile.h>
#include <string>
#include <unordered_map>
#include <vector>

struct VolumeData {
  double timestamp;
  float volume;
};

std::unordered_map<int, double> analyzeVolume(const std::string &path) {
  SF_INFO sfinfo;
  SNDFILE *file = sf_open(path.c_str(), SFM_READ, &sfinfo);
  if (!file) {
    std::cerr << "Error opening file: " << sf_strerror(file) << std::endl;
    return {};
  }

  std::unordered_map<int, double> volume_data;
  double duration = static_cast<double>(sfinfo.frames) / sfinfo.samplerate;

  std::cout << "Duration: " << duration << '\n';

  double samples_per_second =
      static_cast<double>(sfinfo.channels * sfinfo.samplerate);

  std::cout << "SPS: " << samples_per_second << '\n';

  double samples_per_interval =
      samples_per_second * 0.5; // iterate over 0.5 second intervals

  std::cout << "SPI: " << samples_per_interval << '\n';

  int64_t samples = static_cast<int64_t>(
      std::ceil(duration * 2.0)); // count in 0.5 second intervals

  std::cout << "Samples:" << samples << '\n';

  for (int64_t i = 0; i < samples; ++i) {
    double sum = 0.0;
    int count = 0;
    std::vector<float> buffer(
        static_cast<size_t>(samples_per_interval * sfinfo.channels));

    int frames_read = sf_readf_float(file, buffer.data(),
                                     static_cast<size_t>(samples_per_interval));
    count = frames_read * sfinfo.channels;

    for (int j = 0; j < count; ++j) {
      sum += std::fabs(buffer[j]);
    }

    if (count > 0) {
      float volume = static_cast<float>(sum / count);
      volume_data.insert({i / 2.0, volume});
    } else {
      break;
    }
  }

  // Normalize to 1 to 1.2
  float max_volume = 0.0;
  for (const auto &item : volume_data) {
    if (item.second > max_volume) {
      max_volume = item.second;
    }
  }

  for (auto &item : volume_data) {
    item.second = 1.0 + (item.second / max_volume); // Scale to 1 to 1.2
  }

  sf_close(file);
  return volume_data;
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    std::string path = argv[1];
    //    std::cout << "[" << std::endl;

    auto volume_data = analyzeVolume(path);
    for (const auto &item : volume_data) {
      // std::cout << "  { \"timestamp\": " << item.first
      //          << ", \"volume\": " << item.second << " }," << std::endl;
    }

    // std::cout << "]" << std::endl;
  } else {
    std::cout << "Please provide a path as an argument." << std::endl;
  }

  return 0;
}
