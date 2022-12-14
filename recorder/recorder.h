#pragma once

#include "recorder/actions_observer.h"

namespace gunit {
namespace recorder {

struct ScriptGenerationError final : public std::exception {
  explicit ScriptGenerationError(const char* msg);

  const char* what() const _NOEXCEPT override { return _error.c_str(); }

 private:
  std::string _error;
};

class ScriptRecordSession {
 public:
  virtual ~ScriptRecordSession() = default;
  virtual std::string getScript() = 0;
};

using ScriptRecordSessionPtr = std::shared_ptr<ScriptRecordSession>;
ScriptRecordSessionPtr makeLuaRecordingSession(const std::string& moduleName);

}  // namespace recorder
}  // namespace gunit
