//////////////////////////////////////////////////////////////////////////
//
// RCqt Library, (c) 2013-2020, Ryan A. Colyer
//
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
// Worker - An efficient compile-time checked typesafe wrapper for the Qt
// signal/slot architecture, which exposes methods as "Tasks" that can run
// on the same or different threads.
//
// NOTE:  Requires C++11
//
//////////////////////////////////////////////////////////////////////////
//
// A usage example:
//
//   class MyWorker : Worker {
//     public:
//
//     void SetString_Handler(int& val, const string& str) {
//       my_val = val;
//       my_str = str;
//     }
//
//     TaskCaller<int, const string> SetString =
//       TaskHandler(MyWorker::SetString_Handler);
//
//     TaskBlocker<int&, const string&> SetStringBlockingWithRef =
//       TaskHandler(MyWorker::SetString_Handler);
//
//     string MyWorker::GetString_Handler(int& val) {
//       if (val == my_val) {
//         return my_str;
//       }
//     }
//
//     TaskGetter<string, int> GetString =
//       TaskHandler(MyWorker::GetString_Handler);
//
//     TaskCaller<const float> LambdaTask = {this, {[](const float& x) {
//       cout << "This task is defined in-place with a lambda.\n";
//     }}};
//
//     private:
//     string my_str;
//     int my_val;
//   };
//
// For defining tasks, choose between:
// TaskCaller<ParameterTypes...>              // This is asynchronous.
// TaskBlocker<ParameterTypes...>             // This blocks until completion.
// TaskGetter<ReturnType, ParameterTypes...>  // This blocks with return value.
//
// Each caller must then be initialized to a handler with the TaskHandler
// convenience macro, which takes a non-overloaded fully qualified member
// function name (i.e., TaskHandler(ThisClass::MyFunction) ).
//
// While the header declaration method shown above is the standard usage,
// Tasks can in principle be dynamically generated and assigned to any Worker
// and function call via the constructor which takes a pointer to an assigned
// Worker, as well as an RC::Caller functor of the appropriate type, which
// itself can be defined as any static or member function.  The handler will
// then run in the thread of the assigned Worker.
//
// NOTE:  Parameters in the ParameterTypes list can be listed all by value,
// or all by reference.  Passing all by value is the only guaranteed
// thread-safe approach with TaskCaller.  As TaskBlocker and TaskGetter block,
// it is generally safe to use references.
//
// NOTE:  Regardless of whether the ParameterTypes list is all value or all
// reference, the handler function input parameters must always be written as
// a reference.  Also, the Task functor always receives parameters by
// reference.
//
// Efficiency notes:
//
//   When the ParameterTypes are by value, the copy constructor is called
//   once for each parameter, and no assignment operators are called.
//
//   When the ParameterTypes are by reference, no copy constructors or
//   assignment operators are called.
//
//   For TaskGetter's ReturnType as a value, one default constructor is
//   called, as one internal assignment operator.
//
//   When TaskGetter's ReturnType is a reference, no copy constructors or
//   assignment operators are called.
//
////////////////////////////////////////////////////////////////////////////


#ifndef WORKER_H
#define WORKER_H

#include "../RC/RCconfig.h"
#include "../RC/Ptr.h"
#include "../RC/APtr.h"
#include "../RC/Caller.h"
#include "../RC/RTime.h"
#include "../RC/Tuple.h"
#include <map>
#include <QMutex>
#include <QThread>

#ifndef CPP11
#error "Error, C++11 is required."
#endif


#define TaskHandler(func) {this, RC::MakeCaller(this, &func)}

namespace RCqt {
  enum TaskType { AUTOTASK, BLOCKTASK };

  class Worker;
  class WorkerCommand;
  class WorkerThread;

  class WorkerQObject : public QObject {
    Q_OBJECT

    friend Worker;

    public:  WorkerQObject(RC::Ptr<Worker> worker);
    public:
      void CommandEmitter(RC::APtr<WorkerCommand> &cmd,
                          TaskType task_type) const;
    private:  signals:
      void CommandSignal(RC::APtr<WorkerCommand> cmd) const;
      void CommandSignalBlocked(RC::APtr<WorkerCommand> cmd) const;
      void DoFinalExit();
      void TerminateIfEmptySignal();
    private slots:  void CommandSlot(RC::APtr<WorkerCommand> cmd) const;
    private:  void DoTerminate();
              void QueueTerminateIfEmpty();

    public:  void EmitDoneAbort() { emit DoDoneAbort(); }
    private: signals:  void DoDoneAbort();
    private slots:  void DoneAbort();

    public:  void EmitCallExit();
    private: signals:  void DoCallExit();
    private slots:  void CallExit();
                    void TerminateIfEmptySlot();

    private:  RC::Ptr<Worker> worker;
  };

  class Worker {
    friend WorkerQObject;
    friend WorkerCommand;
    friend WorkerThread;

    typedef std::map< RC::Ptr<Worker>, RC::Ptr<Worker> > MapType;
    typedef std::pair< RC::Ptr<Worker>, RC::Ptr<Worker> > MapPair;
    private:


    class AbortLevel {
      public:
      AbortLevel() : level(0), disabled(false) {}
      void Raise() { mutex.lock(); level++; mutex.unlock(); }
      void Lower() { mutex.lock(); level--; mutex.unlock(); }
      bool ShouldAbort() const {
        bool ret;
        mutex.lock();
        ret = disabled || (level > 0);
        mutex.unlock();
        return ret;
      }
      void Disable() { mutex.lock(); disabled = true; mutex.unlock(); }
      bool IsDisabled() const {
        bool ret;
        mutex.lock();
        ret = disabled;
        mutex.unlock();
        return ret;
      }
      private:
      mutable QMutex mutex;
      u64 level;
      bool disabled;
    };


    class TaskCount {
      public:
      TaskCount() : count(0) {}
      void Inc() { mutex.lock(); count++; mutex.unlock(); }
      void Dec() { mutex.lock(); count--; mutex.unlock(); }
      u64 Count() const {
        u64 ret; mutex.lock(); ret=count; mutex.unlock(); return ret; }
      private:
      mutable QMutex mutex;
      u64 count;
    };


    public:

    Worker(bool run_as_new_thread=false);
    ~Worker();

    void CommandEmitter(RC::APtr<WorkerCommand> &cmd,
                        TaskType task_type=AUTOTASK) const;

    private:  // Disallow
    inline Worker(const Worker& other);
    inline Worker& operator=(const Worker& other);
    public:


    static void ExitAllWorkers();
    static void ExitAllLater();

    static int ExecuteWhileWorkers();
    static int ExecuteWhileTasks();

    void Abort() {
      abort_level.Raise();
      worker_qobject.EmitDoneAbort();
    }
    bool ShouldAbort() const { return abort_level.ShouldAbort(); }
    bool KeepGoing() const { return ! ShouldAbort(); }

    bool IsDisabled() const { return abort_level.IsDisabled(); }
    bool IsEnabled() const { return ! IsDisabled(); }

    void Exit() {
      abort_level.Disable();
      worker_qobject.EmitCallExit();
    }
    void ExitWait() {
      while (thread.isRunning()) {
        Exit();
        RC::Time::Sleep(0.020);
      }
    }
    void ExitLater() {
      worker_qobject.EmitCallExit();
    }

    u64 NumTasks() const {
      return task_count.Count();
    }


    private:

    static bool IsMapEmpty();

    QThread thread;

    AbortLevel abort_level;
    TaskCount task_count;
    WorkerQObject worker_qobject;

    static QMutex safe_delete;
    static QMutex worker_map_mutex;
    static MapType worker_map;
    static bool terminate_when_done;
    static WorkerQObject static_qobject;
  };


  class WorkerCommand {
    public:
    WorkerCommand(RC::Ptr<Worker> worker) : worker(worker) {
      worker->task_count.Inc();
    }
    virtual ~WorkerCommand();
    virtual void Run() = 0;
    private:
    WorkerCommand(const WorkerCommand& other);
    WorkerCommand& operator=(const WorkerCommand& other);
    RC::Ptr<Worker> worker;
  };


  // Template for WorkerCommands containing parameters and handlers

  template<class RetType, class... Params>
  class CommandTempl : public WorkerCommand {
    public:
    CommandTempl(RC::Ptr<Worker> worker, RC::Caller<RetType, Params&...>
                 handler, RetType& retval, Params&... params)
      : WorkerCommand(worker)
      , handler(handler)
      , retval(retval)
      , parameters(params...) {
    }

    virtual void Run() {
      retval = parameters.Apply(handler);
    }

    protected:
    u32 x;
    RC::Caller<RetType, Params&...> handler;
    RetType& retval;
    RC::Tuple<Params...> parameters;
  };

  template<class RetType, class... Params>
  class CommandTempl<RetType&, Params...> : public WorkerCommand {
    public:
    CommandTempl(RC::Ptr<Worker> worker, RC::Caller<RetType&, Params&...>
                 handler, RC::Ptr<RetType>& retval, Params&... params)
      : WorkerCommand(worker)
      , handler(handler)
      , retval(retval)
      , parameters(params...) {
    }

    virtual void Run() {
      retval = &(parameters.Apply(handler));
    }

    protected:
    u32 x;
    RC::Caller<RetType&, Params&...> handler;
    RC::Ptr<RetType>& retval;
    RC::Tuple<Params...> parameters;
  };

  template<class... Params>
  class CommandTempl<void, Params...> : public WorkerCommand {
    public:
    CommandTempl(RC::Ptr<Worker> worker, RC::Caller<void, Params&...> handler,
                 Params&... params)
      : WorkerCommand(worker)
      , handler(handler)
      , parameters(params...) {
    }

    virtual void Run() {
      parameters.Apply(handler);
    }

    protected:
    bool tmp;
    u32 x;
    RC::Caller<void, Params&...> handler;
    RC::Tuple<Params...> parameters;
  };


  // The TaskCaller, TaskBlocker, and TaskGetter template classes.

  template<TaskType task_type, class... Params>
  class BaseTaskClass : public RC::CallerBase<void, Params&...> {
    public:
    BaseTaskClass() { }
    BaseTaskClass(const RC::Ptr<Worker>& worker,
                  const RC::Caller<void, Params&...>& handler)
      : worker(worker)
      , handler(handler) {
    }
    virtual void operator()(Params&... params) const {
      RC::APtr<WorkerCommand> cmd =
        new CommandTempl<void, Params...>(worker, handler, params...);
      worker->CommandEmitter(cmd, task_type);
    }
    virtual RC::CallerBase<void, Params&...>* Copy() const {
      return new BaseTaskClass<task_type, Params...>(worker, handler);
    }
    virtual bool IsSet() { return handler.IsSet(); }
    protected:
    RC::Ptr<Worker> worker;
    RC::Caller<void, Params&...> handler;
  };

  template<class... Params>
  class TaskCaller : public BaseTaskClass<AUTOTASK, Params...> {
    public:
    TaskCaller() { }
    TaskCaller(RC::Ptr<Worker> worker, RC::Caller<void, Params&...> handler)
      : BaseTaskClass<AUTOTASK, Params...>(worker, handler) {
    }
    virtual RC::CallerBase<void, Params&...>* Copy() const {
      return new TaskCaller<Params...>(
          BaseTaskClass<AUTOTASK, Params...>::worker,
          BaseTaskClass<AUTOTASK, Params...>::handler);
    }
  };

  template<class... Params>
  class TaskBlocker : public BaseTaskClass<BLOCKTASK, Params...> {
    public:
    TaskBlocker() { }
    TaskBlocker(RC::Ptr<Worker> worker, RC::Caller<void, Params&...> handler)
      : BaseTaskClass<BLOCKTASK, Params...>(worker, handler) {
    }
    virtual RC::CallerBase<void, Params&...>* Copy() const {
      return new TaskBlocker<Params...>(
          BaseTaskClass<BLOCKTASK, Params...>::worker,
          BaseTaskClass<BLOCKTASK, Params...>::handler);
    }
  };

  template<class RetType, class... Params>
  class TaskGetter : public RC::CallerBase<RetType, Params&...> {
    public:
    TaskGetter() { }
    TaskGetter(RC::Ptr<Worker> worker, RC::Caller<RetType, Params&...> handler)
      : worker(worker)
      , handler(handler) {
    }
    virtual RetType operator()(Params&... params) const {
      RetType retval;
      RC::APtr<WorkerCommand> cmd =
        new CommandTempl<RetType, Params...>
              (worker, handler, retval, params...);
      worker->CommandEmitter(cmd, BLOCKTASK);
      return retval;
    }
    virtual RC::CallerBase<RetType, Params&...>* Copy() const {
      return new TaskGetter<RetType, Params...>(worker, handler);
    }
    protected:
    RC::Ptr<Worker> worker;
    RC::Caller<RetType, Params&...> handler;
  };


  template<class RetType, class... Params>
  class TaskGetter<RetType&, Params...>
      : public RC::CallerBase<RetType&, Params&...> {
    public:
    TaskGetter() { }
    TaskGetter(RC::Ptr<Worker> worker, RC::Caller<RetType&, Params&...> handler)
      : worker(worker)
      , handler(handler) {
    }
    virtual RetType& operator()(Params&... params) const {
      RC::Ptr<RetType> retval;
      RC::APtr<WorkerCommand> cmd =
        new CommandTempl<RetType&, Params...>
              (worker, handler, retval, params...);
      worker->CommandEmitter(cmd, BLOCKTASK);
      return *retval;
    }
    virtual RC::CallerBase<RetType&, Params&...>* Copy() const {
      return new TaskGetter<RetType&, Params...>(worker, handler);
    }
    protected:
    RC::Ptr<Worker> worker;
    RC::Caller<RetType&, Params&...> handler;
  };


  // Convenience class for a Worker that spawns a new thread.

  class WorkerThread : public Worker {
    public:
    WorkerThread() : Worker(true) { }

    /// Use AddToThread(this) in the constructor of any QObject derivative
    /// that also has slots.
    void AddToThread(QObject* obj) {
      obj->moveToThread(&thread);
    }
  };

}


#endif // WORKER_H

