/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "absl/types/optional.h"
#include <string>

namespace vi {

struct Options {
  absl::optional<bool> useSimulcast = false;
  absl::optional<bool> useSharingSimulcast = false;
  absl::optional<bool> forceTcp;
  absl::optional<bool> produce = true;
  absl::optional<bool> consume = true;
  absl::optional<bool> forceH264;
  absl::optional<bool> forceVP9;
  absl::optional<std::string> svc;
  absl::optional<bool> datachannel = true;
  absl::optional<std::string> throttleSecret;
  absl::optional<std::string> e2eKey;
};

} // namespace vi
