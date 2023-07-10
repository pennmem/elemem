#include "EEGDisplay.h"
#include "RC/RC.h"
#include "RCmath/MathBits.h"
#include <algorithm>
#include <cmath>

using namespace RC;

namespace CML {
  EEGDisplay::EEGDisplay(int new_width, int new_height) {
    width = new_width;
    height = new_height;

    SetSamplingRate(1000);  // Default 1000Hz
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  }

  EEGDisplay::~EEGDisplay() {
  }


  void EEGDisplay::SetSamplingRate(size_t sampling_rate) {
    data.sampling_rate = sampling_rate;
    data.sample_len = window_seconds*sampling_rate;

    data.data.Resize(num_data_chans);
    for (size_t i=0; i<data.data.size(); i++) {
      data.EnableChan(i);
      data.data[i].Zero();
    }
    data_offset = 0;
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
        (data.sample_len-1);

    for (size_t chan_i = 0; chan_i<eeg_channels.size(); chan_i++) {
      size_t chan = eeg_channels[chan_i].GetDataIndex();
      if (chan > data.data.size()) {
        continue;
      }

      float dmid = 0;
      if (autoscale) {
        float dmax = std::numeric_limits<float>::lowest()/4;
        float dmin = std::numeric_limits<float>::max()/4;
        for (size_t i=0; i<data.data[chan].size(); i++) {
          dmax = std::max(dmax, float(data.data[chan][i]));
          dmin = std::min(dmin, float(data.data[chan][i]));
        }
        float ddiff = dmax-dmin;
        dmid = (dmax-dmin)/2 + dmin;
        if (ddiff < 1) {
          ddiff = 1;
        }
        yscale = float(draw_height) / ddiff;
      }
      else {
        dmid = 0;
        yscale = float(draw_height) / (2*scale_val);
      }

      if (data.data[chan].size() > 0) {
        if (autoscale) {
          SetPen(palette.NormToARGB(0.0f, 0.4f, 1.0f, 1.0f));
        }
        else {
          Color c = palette.GetSpacedColor(chan_i);
          SetPen(palette.NormToARGB(c.r, c.g, c.b, 1.0f));
        }
        QPointF last(qreal(margin_left), qreal(draw_mid - (data.data[chan][0]*yscale)));
        // Skip data for display, slicing at 500Hz.
        size_t inc = data.sample_len / (window_seconds*500);

        for (x = inc; x < data.sample_len; x+=inc) {
          QPointF current(qreal(x * xscale), qreal(draw_mid - ((data.data[chan][x]-dmid) * yscale)));
          painter.drawLine(last, current);
          last = current;
        }
      }
      draw_mid += draw_step;
    }

    if (autoscale) {
      SetPen(palette.NormToARGB(1.0f, 0.0f, 0.2f, 0.7f), 2);
    }
    else {
      SetPen(u32(0xfffffffful));
    }
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
      if (autoscale) {
        SetPen(palette.GetFG_ARGB(0.9f));
      }
      else {
        Color c = palette.GetSpacedColor(chan_i);
        SetPen(palette.NormToARGB(c.r, c.g, c.b, 1.0f));
      }
      painter.drawText(4, draw_mid-draw_height/2, width-2, draw_height,
        Qt::AlignTop | Qt::AlignLeft, eeg_channels[chan_i].GetLabel().ToQString());

      draw_mid += draw_step;
    }
  }

  void EEGDisplay::UpdateData_Handler(RC::APtr<const EEGDataDouble>& new_data_ptr) {
    auto& new_data = new_data_ptr->data;

    // Switch display to new sampling rate.
    if (new_data_ptr->sampling_rate != data.sampling_rate) {
      SetSamplingRate(new_data_ptr->sampling_rate);
    }

    // EEGAcq now guarantees all the same size.  This could be simplified.
    size_t max_len = 0;
    size_t max_chans = std::min(new_data.size(), data.data.size());

    for (size_t c=0; c<max_chans; c++) {
      max_len = std::max(max_len, new_data[c].size());
    }

    for (size_t chan_i = 0; chan_i<eeg_channels.size(); chan_i++) {
      uint32_t c = eeg_channels[chan_i].GetDataIndex();
      if (c >= new_data.size() || c >= data.data.size()) {
        continue;
      }

      size_t i=0;
      size_t data_i = data_offset;
      auto inc_data_i = [&](){
        data_i++;
        if (data_i >= data.data[c].size()) {
          data_i = 0;
        }
      };

      for (; i<new_data[c].size(); i++) {
        data.data[c][data_i] = new_data[c][i];
        inc_data_i();
      }
      for (; i<max_len; i++) {
        data.data[c][data_i] = 0;
        inc_data_i();
      }
    }

    data_offset += max_len;
    data_offset = data_offset % data.sample_len;

    auto tdiff = timer.SinceStart();
    if (tdiff >= 0.03) {
      timer.Start();
      update_cnt++;
      // Alternate frames if running slow.
      if (tdiff < 0.045 || ((update_cnt & 1) == 1)) {
        ReDraw();
      }
    }
  }


  void EEGDisplay::SetChannel_Handler(EEGChan& chan) {
    eeg_channels += chan;
  }


  void EEGDisplay::UnsetChannel_Handler(EEGChan& chan) {
    if (chan.GetDataIndex() < data.data.size()) {
      data.data[chan.GetDataIndex()].Zero();
    }
    for (size_t i=0; i<eeg_channels.size(); i++) {
      if (chan.GetDataIndex() == eeg_channels[i].GetDataIndex()) {
        eeg_channels.Remove(i);
        i--;
      }
    }
  }

  void EEGDisplay::SetAutoScale_Handler(const bool& on) {
    autoscale = on;
  }

  void EEGDisplay::SetScale_Handler(const i64& val) {
    if (val < 1) {
      Throw_RC_Error("Invalid EEG scale value from gui element.");
    }
    scale_val = val / eeg_uV_per_unit;
  }

  void EEGDisplay::SetScaleUnit_Handler(const double& uV_per_unit) {
    eeg_uV_per_unit = uV_per_unit;
  }

  void EEGDisplay::Clear_Handler() {
    for (size_t i=0; i<data.data.size(); i++) {
      data.data[i].Zero();
    }
    eeg_channels.Clear();
  }
}

