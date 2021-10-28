#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "EEGData.h"
#include "RC/APtr.h"
#include "RCqt/Worker.h"

namespace CML {
  using MorletCallback = RCqt::TaskCaller<RC::APtr<const RC::Data1D<double>>>;
  using ClassifierCallback = RCqt::TaskCaller<bool>;

  class Classifier : public RCqt::WorkerThread {
    public:
    virtual ~Classifier() {}

    MorletCallback Classify =
      TaskHandler(Classifier::Classifier_Handler);

    RCqt::TaskCaller<ClassifierCallback> SetCallback =
      TaskHandler(Classifier::SetCallback_Handler);


    protected:
    // TODO: JPB: Rename "Process"?
    virtual bool Classification(RC::APtr<const RC::Data1D<double>>&) = 0; 
    void Classifier_Handler(RC::APtr<const RC::Data1D<double>>&);

    /// Sets the callback for the classification result
    /** @param new_callback The new callback to be set
     */
    void SetCallback_Handler(ClassifierCallback& new_callback);

    ClassifierCallback callback;

    void RegisterCallback_Handler(const RC::RStr& tag,
                                  const ClassifierCallback& callback);
    void RemoveCallback_Handler(const RC::RStr& tag);

    struct TaggedCallback {
      RC::RStr tag;
      ClassifierCallback callback;
    };
    RC::Data1D<TaggedCallback> data_callbacks;
  };
}

#endif // CLASSIFIER_EVEN_ODD_H
