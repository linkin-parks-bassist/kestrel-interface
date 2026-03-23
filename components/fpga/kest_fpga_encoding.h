#ifndef KEST_ENCODE_H_
#define KEST_ENCODE_H_

#define    ZERO_REGISTER_ADDR   2
#define POS_ONE_REGISTER_ADDR  	3
#define NEG_ONE_REGISTER_ADDR   4

#define INSTR_FORMAT_A 0
#define INSTR_FORMAT_B 1

#define NO_SHIFT 255

#define DELAY_FORMAT 8

int kest_fpga_batch_append_block_number(kest_fpga_transfer_batch *batch, int block);

uint32_t kest_block_instr_encode_resource_aware(kest_block *block, const kest_eff_resource_report *res);

int kest_fpga_batch_append_effect(kest_fpga_transfer_batch *batch, kest_effect *effect, kest_eff_resource_report *res, int *pos);
int kest_fpga_transfer_batch_append_effect_register_updates(kest_fpga_transfer_batch *batch, kest_effect_desc *eff, kest_expr_scope *scope, int pos);
int kest_fpga_transfer_batch_append_effect_resource_updates(kest_fpga_transfer_batch *batch, kest_effect_desc *eff, kest_expr_scope *scope, kest_effect_fpga_position *pos);
int kest_fpga_batch_append_effects(kest_fpga_transfer_batch *batch, kest_effect_pll *list, kest_eff_resource_report *res, int *pos);

int kest_fpga_batch_print(kest_fpga_transfer_batch seq);

#endif
