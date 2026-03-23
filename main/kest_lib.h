#ifndef KEST_LIB_MAIN_H_
#define KEST_LIB_MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#define KEST_FILENAME_LEN 128

#define PRINT_TRANSFER_BATCHES

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "kest_alloc.h"
#include "kest_error_codes.h"
#include "kest_linked_list.h"
#include "kest_list.h"
#include "kest_string.h"

#include "kest_lib_cmph.h"

#include "kest_bump_arena.h"
#include "kest_representation.h"
#include "kest_parameter.h"
#include "kest_expr_scope.h"
#include "kest_expression.h"
#include "kest_resource.h"
#include "kest_block.h"
#include "kest_eff_desc.h"
#include "kest_fpga_io.h"
#include "kest_effect.h"
#include "kest_pipeline.h"
#include "kest_preset.h"
//#include "kest_param_update.h"
//#include "kest_status.h"
//#include "kest_effect_enum.h"
//#include "kest_alloc.h"
#include "kest_helper_fn.h"
//#include "kest_i2c.h"
//#include "kest_sgtl5000.h"
//#include "kest_sd.h"
//#include "kest_footswitch.h"
//#include "kest_fpga_comms.h"
//#include "kest_button.h"
//#include "kest_ui.h"
//#include "kest_parameter_widget.h"
//#include "kest_effect_init.h"
//#include "kest_effect_table.h"
//#include "kest_effect_view.h"
//#include "kest_effect_settings.h"
//#include "kest_effect_select.h"
//#include "kest_sequence.h"
//#include "kest_preset_settings.h"
//#include "kest_preset_view.h"
//#include "kest_sequence_view.h"
//#include "kest_settings.h"
//#include "kest_context.h"
//#include "kest_files.h"
//#include "kest_sequence_list.h"
//#include "kest_menu.h"
//#include "kest_lv_log.h"
#include "kest_tokenizer.h"
#include "kest_dictionary.h"
#include "kest_eff_parser.h"
#include "kest_expr_parser.h"
#include "kest_eff_section.h"
#include "kest_asm_parser.h"
#include "kest_reg_format.h"
#include "kest_fixed_point.h"
#include "kest_fpga_encoding.h"
#include "kest_dict_extract.h"
#include "kest_printf.h"

#define PRINTLINES_ALLOWED 0

#ifdef __cplusplus
}
#endif

#endif
