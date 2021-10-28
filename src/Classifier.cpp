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
}
