#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_core_id.h"
#include "ervp_assert.h"
#include "ervp_fakefile.h"

static void check_content(const char* name, const char* expected)
{
  FAKEFILE* fp = ffopen(name, "r");
  assert_must(fp != NULL);
  for(int i=0; expected[i]!=0; i++)
    assert_must(ffgetc(fp) == expected[i]);
  assert_must(ffgetc(fp) == EOF);
  ffclose(fp);
}

int main() {
  if(EXCLUSIVE_ID==0)
  {
    FAKEFILE *fp, *src, *dst;

    fp = ffopen("test.txt", "w");
    ffputc('r', fp);
    ffputc('v', fp);
    ffclose(fp);
    check_content("test.txt", "rv");

    fp = ffopen("test.txt", "a");
    ffputc('x', fp);
    ffclose(fp);
    check_content("test.txt", "rvx");

    assert_must(fakefile_exists("mat_int.csv"));
    src = ffopen("mat_int.csv", "r");
    dst = ffopen("mat_int_copy.csv", "w");
    fakefile_copy(src, dst);
    ffclose(src);
    ffclose(dst);
    assert_must(fakefile_get_size_by_name("mat_int_copy.csv") == fakefile_get_size_by_name("mat_int.csv"));
    src = ffopen("mat_int.csv", "r");
    dst = ffopen("mat_int_copy.csv", "r");
    assert_must(fakefile_diff(src, dst) == 0);
    ffclose(src);
    ffclose(dst);

    printf_must("\n[verify_fileio] PASS\n");
  }
  return 0;
}
