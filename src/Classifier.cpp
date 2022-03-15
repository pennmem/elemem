#include "Classifier.h"
#include "RC/RStr.h"
#include "Handler.h"

namespace CML {
  Classifier::Classifier(RC::Ptr<Handler> hndl,
      RC::APtr<const FeatureWeights> weights)
    : hndl(hndl), weights(weights) {
  }

  /// Handler that registers a callback on the classifier results
  /** @param A (preferably unique) tag/name for the callback
   *  @param The callback on the classifier results
   */
  void Classifier::RegisterCallback_Handler(const RC::RStr& tag,
                                            const ClassifierCallback& callback) {
    RemoveCallback_Handler(tag);
    data_callbacks += TaggedCallback{tag, callback};
  }

  /// Handler that removes a callback on the classifier results.
  /** @param The tag to be removed from the list of callbacks
   *  Note: All tags of the same name will be removed (even if there is more than one)
   */
  void Classifier::RemoveCallback_Handler(const RC::RStr& tag) {
    for (size_t i=0; i<data_callbacks.size(); i++) {
      if (data_callbacks[i].tag == tag) {
        data_callbacks.Remove(i);
        i--;
      }
    }
  }

  /// Handler that starts the classification and reports the result with a callback
  /** @param data The input data to the classifier
   */
  void Classifier::Classifier_Handler(RC::APtr<const EEGPowers>& data, const TaskClassifierSettings& task_classifier_settings) {
    if ( data_callbacks.IsEmpty() ) {
      Throw_RC_Error("Classification callback not set");
    }

    double result = Classification(data);

    for (size_t i=0; i<data_callbacks.size(); i++) {
      data_callbacks[i].callback(result, task_classifier_settings);
    }
  }
}
