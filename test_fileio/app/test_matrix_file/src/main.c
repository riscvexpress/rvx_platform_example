#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_core_id.h"
#include "ervp_fakefile.h"
#include "ervp_special_matrix_op.h"
#include "ervp_matrix_to_csv.h"

int main() {
  if(EXCLUSIVE_ID==0)
  {
    ErvpMatrixInfo* mat;
    //
    mat = matrix_read_csv_file("mat_int.csv", MATRIX_DATATYPE_SINT08, 1);
    matrix_print(mat);
    matrix_write_fixed_element(mat, 0, 0, 2);
    matrix_print(mat);
    matrix_write_csv_file(mat, "mat_int_modified.csv", 1);

    //
    mat = matrix_read_csv_file("mat_float.csv", MATRIX_DATATYPE_FLOAT32, 1);
    matrix_print(mat);
    matrix_write_float_element(mat, 2, 1, 2.0);
    matrix_print(mat);
    matrix_write_csv_file(mat, "mat_float_modified.csv", 1);
  }
	return 0;
}
