#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_core_id.h"

#include "parser.h"

int main() {
  if(EXCLUSIVE_ID==0) {
    parse_network_cfg("test.cfg");
  }

	return 0;
}
