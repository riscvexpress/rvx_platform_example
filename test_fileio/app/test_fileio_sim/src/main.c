#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_core_id.h"
#include "ervp_fakefile.h"

int main() {
  if(EXCLUSIVE_ID==0)
  {
    FAKEFILE* fp;
    fp = ffopen("test.txt", "w");
    ffputc('r', fp);
    ffputc('v', fp);
    ffclose(fp);

    fp = ffopen("test.txt", "a");
    ffputc('x', fp);
    ffclose(fp);
  }
	return 0;
}
