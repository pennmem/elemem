#include "StatusPanel.h"
#include <QHBoxLayout>

namespace CML {
  StatusPanel::StatusPanel() {
    RC::Ptr<QHBoxLayout> pan_layout = new QHBoxLayout();

    subject = new Indicator();
    pan_layout->addWidget(subject);

    session = new Indicator("Session: ");
    pan_layout->addWidget(session);

    trial = new Indicator("Trial: ");
    pan_layout->addWidget(trial);

    state = new Indicator();
    pan_layout->addWidget(state);

    stim_enabled = new Indicator();
    pan_layout->addWidget(stim_enabled);

    stimming = new Indicator("[Stim]");
    stimming->SetColor(stim_off_color);
    pan_layout->addWidget(stimming);

    setLayout(pan_layout);
    
    Clear_Handler();

    stimming_timer.setTimerType(Qt::PreciseTimer);
    stimming_timer.setSingleShot(true);
    connect(&stimming_timer, &QTimer::timeout, this,
        &StatusPanel::StimmingDone);
  }

  void StatusPanel::SetStimList_Handler(const bool& stim_list) {
    stim_enabled->Set(stim_list ? "Stim List" : "No Stim");
  }

  void StatusPanel::SetEvent_Handler(const RC::RStr& event) {
    state->Set(event);
  }

  void StatusPanel::SetStimming_Handler(const uint32_t& duration_us) {
    stimming->SetColor(stim_on_color);
    stimming_timer.start(duration_us/1000);
  }

  void StatusPanel::StimmingDone() {
    stimming->SetColor(stim_off_color);
  }

  void StatusPanel::SetSession_Handler(const int64_t& session_num) {
    session->Set(session_num);
  }

  void StatusPanel::SetTrial_Handler(const int64_t& trial_num) {
    trial->Set(trial_num);
  }

  void StatusPanel::Clear_Handler() {
    stim_enabled->Set("");
    state->Set("DISCONNECTED");
    session->Set("");
    trial->Set("");
  }
}

