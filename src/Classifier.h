#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "EEGData.h"
#include "EEGPowers.h"
#include "TaskClassifierSettings.h"
#include "RC/APtr.h"
#include "RCqt/Worker.h"

namespace CML {
  class Handler;

  using FeatureCallback = RCqt::TaskCaller<RC::APtr<const EEGPowers>, const TaskClassifierSettings>;
  using ClassifierCallback = RCqt::TaskCaller<const double, const TaskClassifierSettings>;

  class Classifier : public RCqt::WorkerThread {
    public:
    Classifier(RC::Ptr<Handler> hndl) : hndl(hndl) {}
    virtual ~Classifier() {}

    FeatureCallback Classify =
      TaskHandler(Classifier::Classifier_Handler);

    RCqt::TaskCaller<const RC::RStr, const ClassifierCallback> RegisterCallback =
      TaskHandler(Classifier::RegisterCallback_Handler);


    protected:
    virtual double Classification(RC::APtr<const EEGPowers>&) = 0;
    void Classifier_Handler(RC::APtr<const EEGPowers>&, const TaskClassifierSettings&);

    void RegisterCallback_Handler(const RC::RStr& tag,
                                  const ClassifierCallback& callback);
    void RemoveCallback_Handler(const RC::RStr& tag);

    struct TaggedCallback {
      RC::RStr tag;
      ClassifierCallback callback;
    };
    RC::Data1D<TaggedCallback> data_callbacks;

    RC::Ptr<Handler> hndl;
  };
}

#endif // CLASSIFIER_H
