#include "Classifier.h"

namespace CML {
  /// Sets the callback for the classification result
  /** @param new_callback The new callback to be set
   */
  void Classifier::SetCallback_Handler(ClassifierCallback& new_callback) { callback = new_callback; }

  void Classifier::RegisterCallback_Handler(const RC::RStr& tag,
                                            const ClassifierCallback& callback) {
    RemoveCallback_Handler(tag);
    data_callbacks += TaggedCallback{tag, callback};
  }


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
  void Classifier::Classifier_Handler(RC::APtr<const RC::Data1D<double>>& data) {
    RC_DEBOUT(RC::RStr("Classifier_Handler\n\n"));
    if ( ! callback.IsSet() ) {
      return;
    }

    bool result = Classification(data);

    callback(result);

    for (size_t i=0; i<data_callbacks.size(); i++) {
      data_callbacks[i].callback(result);
    }
  }
}
