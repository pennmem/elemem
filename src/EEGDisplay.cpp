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

    int num_channels = RC::CappedCast<int>(eeg_channels.size());
    int draw_height = (height - margin_top - margin_bot -
                       margin_betw * num_channels) / num_channels;
    int draw_step = draw_height + margin_betw;
    int draw_mid_first = margin_top + draw_step/2;
    int draw_mid = draw_mid_first;
    xscale = (float(width - 1e-4 - margin_left - margin_right)) /
        (data_samples-1);

    for (size_t chan_i = 0; chan_i<eeg_channels.size(); chan_i++) {
      size_t chan = eeg_channels[chan_i].channel;
      if (chan > data.size()) {
        continue;
      }
      float dmax = std::numeric_limits<float>::lowest()/4;
      float dmin = std::numeric_limits<float>::max()/4;
      for (size_t i=0; i<data[chan].size(); i++) {
        dmax = std::max(dmax, float(data[chan][i]));
        dmin = std::min(dmin, float(data[chan][i]));
      }
      float ddiff = 2*std::max(std::abs(dmin), std::abs(dmax));
      ddiff = std::max(ddiff, dmax-dmin);
      if (ddiff < 1) {
        ddiff = 1;
      }
      yscale = float(draw_height) / ddiff;

      if (data[chan].size() > 0) {
        SetPen(palette.NormToARGB(0.0f, 0.4f, 1.0f, 1.0f));
        QPointF last(qreal(margin_left), qreal(draw_mid - (data[chan][0]*yscale)));
        for (x = 1; x < data_samples; x++) {
          QPointF current(qreal(x * xscale), qreal(draw_mid - (data[chan][x] * yscale)));
          painter.drawLine(last, current);
          last = current;
        }
      }
      draw_mid += draw_step;
    }

    SetPen(palette.NormToARGB(1.0f, 0.0f, 0.2f, 0.7f), 2);
    QPointF offset_bot{qreal(data_offset * xscale),
          qreal(height - margin_bot)};
    QPointF offset_top{qreal(data_offset * xscale),
          qreal(margin_top)};
    painter.drawLine(offset_bot, offset_top);

    draw_mid  = draw_mid_first;
    for (size_t chan_i = 0; chan_i<eeg_channels.size(); chan_i++) {

      QFont font = painter.font();
      font.setPixelSize(14);
      painter.setFont(font);
      SetPen(palette.GetFG_ARGB(0.9f));
      painter.drawText(4, draw_mid-draw_height/2, width-2, draw_height,
        Qt::AlignTop | Qt::AlignLeft, eeg_channels[chan_i].label.ToQString());

      draw_mid += draw_step;
    }
  }

  void EEGDisplay::UpdateData_Handler(RC::APtr<const EEGData>& new_data_ptr) {
    auto& new_data = *new_data_ptr;

    // EEGAcq now guarantees all the same size.  This could be simplified.
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

    // Alternate frames if running slow.
    auto tdiff = timer.SinceStart();
    timer.Start();
    update_cnt++;
    if (tdiff > 0.03) {
      if ((update_cnt & 1) == 1) {
        ReDraw();
      }
    }
    else {
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

  void EEGDisplay::Clear_Handler() {
    for (size_t i=0; i<data.size(); i++) {
      data[i].Zero();
    }
    eeg_channels.Clear();
  }
}

