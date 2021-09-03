#include "ExperOPS.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "StatusPanel.h"
#include "RC/Data1D.h"

using namespace RC;

namespace CML {
  ExperOPS::ExperOPS(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
    AddToThread(this);
  }

  ExperOPS::~ExperOPS() {
    Stop_Handler();
  }

  void ExperOPS::SetStimProfiles_Handler(
        const RC::Data1D<CSStimProfile>& new_stim_profiles) {
    stim_profiles = new_stim_profiles;

    // Build all the stim grid events with ISI between.
    exp_events.Clear();
    for (size_t p=0; p<stim_profiles.size(); p++) {
      const CSStimChannel& stim_info = stim_profiles[p][0];

      // Stim events for each grid cell's stim profile.
      for (size_t e=0; e<ops_specs.num_stim_trials; e++) {
        ExpEvent ev;
        ev.active_ms = stim_info.duration/1000;
        // Add ISI
        ev.event_ms = ev.active_ms +
          rng.GetRange(ops_specs.intertrial_range_ms[0],
              ops_specs.intertrial_range_ms[1]);

        auto prefunc = MakeFunctor<void>(
            hndl->stim_worker.ConfigureStimulation.ToCaller().
            Bind(stim_profiles[p]));
        ev.pre_event =
          MakeCaller(this, &ExperOPS::DoConfigEvent).Bind(prefunc);

        ev.event = MakeFunctor<void>(
            MakeCaller(this, &ExperOPS::DoStimEvent).Bind(
            hndl->stim_worker.Stimulate.ToCaller()));

        exp_events += ev;
      }
    }

    // Build the sham events.
    for (size_t e=0; e<ops_specs.num_sham_trials; e++) {
      ExpEvent ev;
      ev.active_ms = 0;
      ev.event_ms = ops_specs.sham_duration_ms;
      ev.event = MakeCaller(this, &ExperOPS::DoShamEvent);

      exp_events += ev;
    }

    // Randomize all the experiment events.
    exp_events.Shuffle();
  }


  void ExperOPS::DoConfigEvent(RC::Caller<>event) {
    status_panel->SetEvent("PRESET");
    event();
  }


  void ExperOPS::DoStimEvent(RC::Caller<> event) {
    status_panel->SetEvent("STIM");
    event();
  }


  void ExperOPS::DoShamEvent() {
    JSONFile sham_event = MakeResp("SHAM");
    sham_event.Set(ops_specs.sham_duration_ms, "data", "duration");
    hndl->event_log.Log(sham_event.Line());
    status_panel->SetEvent("SHAM");
  }


  void ExperOPS::Start_Handler() {
    BeAllocatedTimer();
    cur_ev = 0;
    pre_ev = true;
    event_time = 0;
    next_event_time = 2000;

    // Calculate total run time.
    uint64_t total_time = next_event_time;
    for (size_t i=0; i<exp_events.size(); i++) {
      total_time += exp_events[i].event_ms;
    }
    uint64_t seconds = (total_time + 500)/1000;
    uint64_t minutes = seconds / 60;
    seconds = seconds % 60;

    // Confirm window for run time.
    if (!ConfirmWin(RC::RStr("Total run time will be ") + minutes + " min. "
          + seconds + " sec.", "Session Duration")) {
      hndl->StopExperiment();
      return;
    }

    exp_start = RC::Time::Get();

    JSONFile startlog = MakeResp("START");
    hndl->event_log.Log(startlog.Line());

    RunEvent();
  }


  void ExperOPS::Stop_Handler() {
    if (timer.IsSet()) {
      timer->stop();
    }

    JSONFile stoplog = MakeResp("EXIT");
    hndl->event_log.Log(stoplog.Line());
  }


  void ExperOPS::InternalStop() {
    Stop_Handler();
    hndl->StopExperiment();
  }


  void ExperOPS::RunEvent() {
    if (cur_ev >= exp_events.size()) {
      InternalStop();
      return;
    }

    auto& cur = exp_events[cur_ev];

    if (pre_ev) {
      if (cur.pre_event.IsSet()) {
        cur.pre_event();
      }

      pre_ev = false;
      // Do regular event.
      TriggerAt(next_event_time);
    }
    else {
      if (cur.event.IsSet()) {
        cur.event();
      }

      pre_ev = true;
      cur_ev++;

      next_event_time = event_time + cur.event_ms;

      // Do pre-event.
      if (cur_ev < exp_events.size()) {
        uint64_t pre_del = cur.active_ms + (cur.event_ms - cur.active_ms)/3;
        TriggerAt(event_time + pre_del);
      }
      // Final Stop_Handler event.
      else {
        TriggerAt(next_event_time);
      }
    }
  }

  void ExperOPS::TriggerAt(uint64_t target_ms) {
    f64 cur_time = RC::Time::Get();
    uint64_t current_time_ms = uint64_t(1000*(cur_time - exp_start)+0.5);

    event_time = target_ms;

    uint64_t delay;
    if (target_ms <= current_time_ms) {
      delay = 0;
    } else {
      delay = target_ms - current_time_ms;
    }

    timer->start(delay);
  }


  void ExperOPS::BeAllocatedTimer() {
    if (timer.IsNull()) {
      timer = new QTimer();
      timer->setTimerType(Qt::PreciseTimer);
      timer->setSingleShot(true);

      QObject::connect(timer.Raw(), &QTimer::timeout, this,
                     &ExperOPS::RunEvent);
    }
  }

}

