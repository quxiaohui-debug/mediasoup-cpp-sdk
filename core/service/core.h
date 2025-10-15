/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once
#include <string>
namespace vi {

class Core {
public:
  Core();

  static void init(const std::string &log_path);

  static void destroy();
};

} // namespace vi
