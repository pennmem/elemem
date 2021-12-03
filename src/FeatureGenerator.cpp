#include "FeatureGenerator.h"

namespace CML {
  /// Handler that sets the callback on the feature generator results
  /** @param The callback on the classifier results
   */
  void FeatureGenerator::SetCallback_Handler(FeatureCallback &new_callback) {
    callback = new_callback;  
  }  
}
