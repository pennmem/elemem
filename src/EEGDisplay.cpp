#include "EEGDisplay.h"
#include "RC/RC.h"
#include <algorithm>

using namespace RC;

namespace CML {
  EEGDisplay::EEGDisplay(int new_width, int new_height) {
    width = new_width;
    height = new_height;

    data.Resize(num_data_chans);
    for (size_t i=0; i<data.size(); i++) {
      data[i].Resize(data_samples);
      data[i].Zero();
    }
  }


  EEGDisplay::~EEGDisplay() {
  }


  void EEGDisplay::DrawBackground() {
  }


  void EEGDisplay::DrawOnTop() {
    if (eeg_channels.size() == 0) {
      return;
    }

    size_t x;
    float xscale, yscale;
    int margin_left = 2;
    int margin_right = 2;
    int margin_top = 2;
    int margin_bot = 2;
    int margin_betw = 5;

    // TODO correct scale for this.
    float yrange = 400.0f; // up and down

    int num_channels = RC::CappedCast<int>(eeg_channels.size());
    int draw_height = (height - margin_top - margin_bot -
                       margin_betw * num_channels) / num_channels;
    int draw_step = draw_height + margin_betw;
    int draw_mid = height - margin_top - draw_height + draw_step/2;
    xscale = (float(width - 1 - margin_left - margin_right)) /
        (data.size()-1);

    for (size_t chan_i = 0; chan_i<eeg_channels.size(); chan_i++) {
      yscale = float(draw_height) / (2*yrange);
      size_t chan = eeg_channels[chan_i].channel;

      if (data[chan].size() > 0) {
        SetPen(palette.NormToARGB(0.0f, 0.4f, 1.0f, 1.0f));
        QPointF last(qreal(margin_left), qreal(draw_mid - (data[chan][0]*yscale)));
        for (x = 1; x < data.size(); x++) {
          QPointF current(qreal(x * xscale), qreal(draw_mid - (data[chan][x] * yscale)));
          painter.drawLine(last, current);
          last = current;
        }
      }

      draw_mid -= draw_step;
    }

    //CornerText(RStr("max: ") + max);
  }

  void EEGDisplay::UpdateData_Handler(EEGData& new_data) {
    static size_t count = 0;
    static auto last_time = RC::Time::Get();
    if (count % 20 == 0) {
      std::cout << "Updating data, " << count << "th run.\n";
      std::cout << "Time interval: " << (RC::Time::Get()-last_time) << "\n";
      std::cout << "new_data[0].size() == " << new_data[0].size() << std::endl;

    }
    count++;
    last_time = RC::Time::Get();

    size_t max_len = 0;
    size_t max_chans = std::min(new_data.size(), data.size());

    for (size_t c=0; c<max_chans; c++) {
      max_len = std::max(max_len, new_data[c].size());
    }

    for (size_t chan_i = 0; chan_i<eeg_channels.size(); chan_i++) {
      uint32_t c = eeg_channels[chan_i].channel;
      if (c >= new_data.size() || c >= data.size()) {
        continue;
      }

      size_t i=0;
      size_t data_i = data_offset;
      auto inc_data_i = [&](){
        data_i++;
        if (data_i >= data[c].size()) {
          data_i = 0;
        }
      };

      for (; i<new_data[c].size(); i++) {
        data[c][data_i] = new_data[c][i];
        inc_data_i();
      }
      for (; i<max_len; i++) {
        data[c][data_i] = 0;
        inc_data_i();
      }
    }

    data_offset += max_len;
    data_offset = data_offset % data_samples;

    if (count % 2 == 0) {
      ReDraw();
    }
  }


  void EEGDisplay::SetChannel_Handler(EEGChan& chan) {
    eeg_channels += chan;
  }


  void EEGDisplay::UnsetChannel_Handler(EEGChan& chan) {
    if (chan.channel < data.size()) {
      data[chan.channel].Zero();
    }
    for (size_t i=0; i<eeg_channels.size(); i++) {
      if (chan.channel == eeg_channels[i].channel) {
        eeg_channels.Remove(i);
        i--;
      }
    }
  }
}

