//////////////////////////////////////////////////////////////////////////
//
// RCnix Library, (c) 2013-2019, Ryan A. Colyer
//
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
//////////////////////////////////////////////////////////////////////////

#include "Worker.h"
#include <QCoreApplication>
#include <QMetaType>
#include <QObject>


namespace RCqt {
  QMutex Worker::safe_delete;
  QMutex Worker::worker_map_mutex;
  Worker::MapType Worker::worker_map;
  bool Worker::terminate_when_done = false;
  WorkerQObject Worker::static_qobject(NULL);

  WorkerQObject::WorkerQObject(RC::Ptr<Worker> worker)
    : worker(worker) {

    connect(this, &WorkerQObject::DoDoneAbort, this, &WorkerQObject::DoneAbort,
            Qt::QueuedConnection);
    connect(this, &WorkerQObject::DoCallExit, &WorkerQObject::CallExit);
    qRegisterMetaType< RC::APtr<WorkerCommand> >("RC::APtr<WorkerCommand>");
    connect(this, &WorkerQObject::CommandSignal,
            &WorkerQObject::CommandSlot);
    // Note.  The commands replace this blocked with an auto, working as
    // a direct, if called from the worker_qobject's thread.
    connect(this, &WorkerQObject::CommandSignalBlocked,
            this, &WorkerQObject::CommandSlot,
            Qt::BlockingQueuedConnection);

    connect(this, &WorkerQObject::TerminateIfEmptySignal,
            this, &WorkerQObject::TerminateIfEmptySlot, Qt::QueuedConnection);
  }


  Worker::Worker(bool run_as_new_thread)
    : worker_qobject(this) {

    worker_map_mutex.lock();
    worker_map.insert(MapPair(this, this));
    worker_map_mutex.unlock();

    if (run_as_new_thread) {
      worker_qobject.moveToThread(&thread);
      thread.start();
    }
  }


  Worker::~Worker() {
    ExitWait();

    worker_map_mutex.lock();
    worker_map.erase(this);
    worker_map_mutex.unlock();
    
    safe_delete.lock();  // Do not destruct anything while iterating.
    safe_delete.unlock();
  }


  void Worker::CommandEmitter(RC::APtr<WorkerCommand> &cmd,
                              TaskType task_type) const {
    worker_qobject.CommandEmitter(cmd, task_type);
  }


  void Worker::ExitAllWorkers() {
    bool empty = false;
    while (!empty) {
      MapType local_map;
      safe_delete.lock();  // Do not destruct anything while iterating.
      worker_map_mutex.lock();     // LOCK
      local_map = worker_map;
      worker_map_mutex.unlock();   // UNLOCK

      empty = local_map.empty();

      MapType::iterator it;
      for (it = local_map.begin(); it != local_map.end(); ++it) {
        it->second->Exit();
      }
      safe_delete.unlock();

      if (!empty) {
        RC::Time::Sleep(0.020);
      }
    }
  }


  void Worker::ExitAllLater() {
    MapType local_map;
    safe_delete.lock();  // Do not destruct anything while iterating.
    worker_map_mutex.lock();     // LOCK
    local_map = worker_map;
    terminate_when_done = true;
    worker_map_mutex.unlock();   // UNLOCK

    MapType::iterator it;
    for (it = local_map.begin(); it != local_map.end(); ++it) {
      it->second->ExitLater();
    }

    bool do_terminate = IsMapEmpty();
    safe_delete.unlock();

    if (do_terminate) {
      static_qobject.DoTerminate();
    }
  }


  int Worker::ExecuteWhileWorkers() {
    static_qobject.QueueTerminateIfEmpty();
    return QCoreApplication::instance()->exec();
  }


  int Worker::ExecuteWhileTasks() {
    ExitAllLater();
    return ExecuteWhileWorkers();
  }


  bool Worker::IsMapEmpty() {
    worker_map_mutex.lock();     // LOCK
    bool do_terminate = false;
    if (worker_map.empty()) {
      do_terminate = true;
    }
    worker_map_mutex.unlock();   // UNLOCK
    return do_terminate;
  }


  void WorkerQObject::CommandEmitter(RC::APtr<WorkerCommand> &cmd,
                                     TaskType task_type) const {
    switch(task_type) {
      case AUTOTASK:  emit CommandSignal(cmd);  break;
      case BLOCKTASK:
        if (QThread::currentThread() == worker->worker_qobject.thread()) {
          emit CommandSignal(cmd);
        }
        else {
          emit CommandSignalBlocked(cmd);
        }
        break;
      default:  Throw_RC_Error("Invalid emitter case");
    }
  }


  void WorkerQObject::CommandSlot(RC::APtr<WorkerCommand> cmd) const {
    if (worker->KeepGoing()) {
      cmd->Run();
    }
  }


  void WorkerQObject::DoneAbort() {
    worker->abort_level.Lower();
  }


  void WorkerQObject::CallExit() {
    bool do_terminate = false;
    worker->abort_level.Disable();
    Worker::worker_map_mutex.lock();
    Worker::worker_map.erase(worker);
    if (Worker::terminate_when_done && Worker::worker_map.empty()) {
      do_terminate = true;
    }
    Worker::worker_map_mutex.unlock();
    worker->thread.exit(0);
    if (do_terminate) {
      DoTerminate();
    }
  }


  void WorkerQObject::DoTerminate() {
    connect(this, &WorkerQObject::DoFinalExit,
            QCoreApplication::instance(), &QCoreApplication::quit);
    emit DoFinalExit();
  }


  void WorkerQObject::TerminateIfEmptySlot() {
    if (Worker::IsMapEmpty()) {
      DoTerminate();
    }
  }


  void WorkerQObject::QueueTerminateIfEmpty() {
    emit TerminateIfEmptySignal();
  }

  
  void WorkerQObject::EmitCallExit() {
    if (thread()->isRunning()) {
      emit DoCallExit();
    }
    else {
      // Conditional does not need to be atomic.
      // In all sensible usage, should only apply in the ExitAll loop for
      // Worker objects running in a thread that has alread exited.
      CallExit();
    }
  }
}

