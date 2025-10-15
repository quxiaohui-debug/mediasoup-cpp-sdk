/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/
#pragma warning(disable : 4996)
#include "signaling_models.h"

namespace signaling {

void to_json(nlohmann::json &j, const Sharing &st) {
  j["trackName"] = st.trackName;
}

void from_json(const nlohmann::json &j, Sharing &st) {
  if (j.contains("trackName")) {
    j.at("trackName").get_to(st.trackName);
  }
}

void to_json(nlohmann::json &j, const SharingAppData &st) {
  j["sharing"] = st.sharing;
}

void from_json(const nlohmann::json &j, SharingAppData &st) {
  if (j.contains("sharing")) {
    j.at("sharing").get_to(st.sharing);
  }
}

} // namespace signaling
