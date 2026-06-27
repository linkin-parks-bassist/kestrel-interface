#include "kest_int.h"

IMPLEMENT_LIST(kest_fpga_command);

kest_fpga_command kest_fpga_command_begin_program()
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_BEGIN_PROGRAM;
	return cmd;
}

kest_fpga_command kest_fpga_command_end_program()
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_END_PROGRAM;
	return cmd;
}

kest_fpga_command kest_fpga_command_write_block_instr(int block, uint32_t instr)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_WRITE_BLOCK_INSTR;
	cmd.data_1.block = block;
	cmd.data_2.instr = instr;
	return cmd;
}

kest_fpga_command kest_fpga_command_write_block_reg_0(int block, float val, int format)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_WRITE_BLOCK_REG_0;
	cmd.data_1.block = block;
	cmd.val = val;
	cmd.format = format;
	return cmd;
}

kest_fpga_command kest_fpga_command_write_block_reg_1(int block, float val, int format)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_WRITE_BLOCK_REG_1;
	cmd.data_1.block = block;
	cmd.val = val;
	cmd.format = format;
	return cmd;
}

kest_fpga_command kest_fpga_command_update_block_reg_0(int block, float val, int format)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_UPDATE_BLOCK_REG_0;
	cmd.data_1.block = block;
	cmd.val = val;
	cmd.format = format;
	return cmd;
}

kest_fpga_command kest_fpga_command_update_block_reg_1(int block, float val, int format)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_UPDATE_BLOCK_REG_1;
	cmd.data_1.block = block;
	cmd.val = val;
	cmd.format = format;
	return cmd;
}

kest_fpga_command kest_fpga_command_commit_reg_updates()
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_COMMIT_REG_UPDATES;
	return cmd;
}

kest_fpga_command kest_fpga_command_alloc_delay(int size, int delay)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_ALLOC_DELAY;
	cmd.data_1.size = size;
	cmd.data_2.delay = delay;
	return cmd;
}

kest_fpga_command kest_fpga_command_alloc_filter(int format, int order_ff, int order_fb)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_ALLOC_FILTER;
	cmd.data_1.order_ff = order_ff;
	cmd.data_2.order_fb = order_fb;
	cmd.format = format;
	return cmd;
}

kest_fpga_command kest_fpga_command_write_filter_coef(int handle, int coef, float val, int format)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_WRITE_FILTER_COEF;
	cmd.data_1.handle = handle;
	cmd.data_2.coef = coef;
	cmd.val = val;
	cmd.format = format;
	return cmd;
}

kest_fpga_command kest_fpga_command_update_filter_coef(int handle, int coef, float val, int format)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_UPDATE_FILTER_COEF;
	cmd.data_1.handle = handle;
	cmd.data_2.coef = coef;
	cmd.val = val;
	cmd.format = format;
	return cmd;
}

kest_fpga_command kest_fpga_command_commit_filter_coefs(int handle)
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_COMMIT_FILTER_COEF;
	cmd.data_1.handle = handle;
	return cmd;
}

kest_fpga_command kest_fpga_command_enable_tail()
{
	kest_fpga_command cmd = {0};
	cmd.type = COMMAND_ENABLE_TAIL;
	return cmd;
}

int kest_fpga_command_list_append_encoded(kest_fpga_command_list *cmds, kest_fpga_transfer_batch *batch)
{
	if (!cmds || !batch)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	for (size_t i = 0; i < cmds->count && ret_val == NO_ERROR; i++)
		ret_val = kest_fpga_command_append_encoded(cmds->entries[i], batch);
	
	return ret_val;
}

int kest_fpga_command_append_encoded(kest_fpga_command cmd, kest_fpga_transfer_batch *batch)
{
	if (!batch)
		return ERR_NULL_PTR;
	
	int ret_val = kest_fpga_batch_append(batch, (uint8_t)cmd.type);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	switch (cmd.type)
	{
		case COMMAND_WRITE_BLOCK_INSTR:
			if ((ret_val = kest_fpga_batch_append_block_number(batch, cmd.data_1.block) != NO_ERROR)) break;
			if ((ret_val = kest_fpga_batch_append_32          (batch, cmd.data_2.instr) != NO_ERROR)) break;
			break;
			
		case COMMAND_WRITE_BLOCK_REG_0:
		case COMMAND_WRITE_BLOCK_REG_1:
		case COMMAND_UPDATE_BLOCK_REG_0:
		case COMMAND_UPDATE_BLOCK_REG_1:
			if ((ret_val = kest_fpga_batch_append_block_number(batch, cmd.data_1.block   ) != NO_ERROR)) break;
			if ((ret_val = kest_fpga_batch_append_float       (batch, cmd.val, cmd.format) != NO_ERROR)) break;
			break;
			
		case COMMAND_ALLOC_DELAY:
			
			if ((ret_val = kest_fpga_batch_append_24(batch, cmd.data_1.size ) != NO_ERROR)) break;
			if ((ret_val = kest_fpga_batch_append_24(batch, cmd.data_2.delay) != NO_ERROR)) break;
			break;
			
		case COMMAND_SET_INPUT_GAIN:
			if ((ret_val = kest_fpga_batch_append_float(batch, powf(10, cmd.val / 20.0), KEST_FPGA_GAIN_FORMAT) != NO_ERROR)) break;
			break;
			
		case COMMAND_SET_OUTPUT_GAIN:
			if ((ret_val = kest_fpga_batch_append_float(batch, powf(10, cmd.val / 20.0), KEST_FPGA_GAIN_FORMAT) != NO_ERROR)) break;
			break;
			
		case COMMAND_ALLOC_FILTER:
			if ((ret_val = kest_fpga_batch_append(batch, cmd.format & 0xFF         ) != NO_ERROR)) break;
			if ((ret_val = kest_fpga_batch_append(batch, cmd.data_1.order_ff & 0xFF) != NO_ERROR)) break;
			if ((ret_val = kest_fpga_batch_append(batch, cmd.data_2.order_fb & 0xFF) != NO_ERROR)) break;
			break;
			
		case COMMAND_WRITE_FILTER_COEF:
		case COMMAND_UPDATE_FILTER_COEF:
			if ((ret_val = kest_fpga_batch_append   (batch, cmd.data_1.handle & 0xFF) != NO_ERROR)) break;
			#if   KEST_FPGA_FILTER_COEF_INDEX_BYTES == 1
			if ((ret_val = kest_fpga_batch_append   (batch, cmd.data_2.coef   & 0xFF) != NO_ERROR)) break;
			#elif KEST_FPGA_FILTER_COEF_INDEX_BYTES == 2
			if ((ret_val = kest_fpga_batch_append_16(batch, cmd.data_2.coef   & 0xFFFF) != NO_ERROR)) break;
			#endif
			if ((ret_val = kest_fpga_batch_append_float_filter_width(batch, cmd.val, cmd.format) != NO_ERROR)) break;
			break;
			
		case COMMAND_COMMIT_FILTER_COEF:
			if ((ret_val = kest_fpga_batch_append(batch, cmd.data_1.handle & 0xFF) != NO_ERROR)) break;
			break;
	}
	
	return ret_val;
}

char *kest_fpga_command_to_string(int command)
{
	switch (command)
	{
		case COMMAND_BEGIN_PROGRAM: 		return "COMMAND_BEGIN_PROGRAM";
		case COMMAND_WRITE_BLOCK_INSTR: 	return "COMMAND_WRITE_BLOCK_INSTR";
		case COMMAND_WRITE_BLOCK_REG_0: 	return "COMMAND_WRITE_BLOCK_REG_0";
		case COMMAND_WRITE_BLOCK_REG_1: 	return "COMMAND_WRITE_BLOCK_REG_1";
		case COMMAND_ALLOC_DELAY: 			return "COMMAND_ALLOC_DELAY";
		case COMMAND_END_PROGRAM: 			return "COMMAND_END_PROGRAM";
		case COMMAND_SET_INPUT_GAIN: 		return "COMMAND_SET_INPUT_GAIN";
		case COMMAND_SET_OUTPUT_GAIN: 		return "COMMAND_SET_OUTPUT_GAIN";
		case COMMAND_UPDATE_BLOCK_REG_0: 	return "COMMAND_UPDATE_BLOCK_REG_0";
		case COMMAND_UPDATE_BLOCK_REG_1:	return "COMMAND_UPDATE_BLOCK_REG_1";
		case COMMAND_COMMIT_REG_UPDATES: 	return "COMMAND_COMMIT_REG_UPDATES";
		case COMMAND_ALLOC_FILTER: 			return "COMMAND_ALLOC_FILTER";
		case COMMAND_WRITE_FILTER_COEF: 	return "COMMAND_WRITE_FILTER_COEF";
		case COMMAND_UPDATE_FILTER_COEF: 	return "COMMAND_UPDATE_FILTER_COEF";
		case COMMAND_COMMIT_FILTER_COEF: 	return "COMMAND_COMMIT_FILTER_COEF";
		case COMMAND_READOUT: 				return "COMMAND_READOUT";
		case COMMAND_CLEAR_TIMEOUT_FLAG: 	return "COMMAND_CLEAR_TIMEOUT_FLAG";
		case COMMAND_CLEAR_BAD_FLAG: 		return "COMMAND_CLEAR_BAD_FLAG";
		case COMMAND_CLEAR_CMD_ERR_FLAG: 	return "COMMAND_CLEAR_CMD_ERR_FLAG";
		case COMMAND_READ: 					return "COMMAND_READ";
		case COMMAND_ENABLE_TAIL: 			return "COMMAND_ENABLE_TAIL";
	}
	
	return "UNKNOWN";
}


int kest_fpga_command_to_string_(kest_fpga_command cmd, kest_string *str)
{
	kest_string_append_str(str, kest_fpga_command_to_string(cmd.type));
	
	switch (cmd.type)
	{
		case COMMAND_WRITE_BLOCK_INSTR:
			kest_string_appendf(str, "(block = %d, instr = ", cmd.data_1.block);
			kest_instr_print(str, cmd.data_2.instr);
			kest_string_append(str, ')');
			break;
			
		case COMMAND_WRITE_BLOCK_REG_0:
		case COMMAND_WRITE_BLOCK_REG_1:
		case COMMAND_UPDATE_BLOCK_REG_0:
		case COMMAND_UPDATE_BLOCK_REG_1:
			kest_string_appendf(str, "(block = %d, val = %s%.04f)", cmd.data_1.block,
				cmd.val < 0 ? "" : " ", cmd.val);
			break;
			
		case COMMAND_ALLOC_DELAY:
			kest_string_appendf(str, "(size = %d, delay = %d)", cmd.data_1.size, cmd.data_2.delay);
			break;
			
		case COMMAND_SET_INPUT_GAIN:
			kest_string_appendf(str, "(dB = %s%.04f)", cmd.val < 0 ? "" : " ", cmd.val);
			break;
			
		case COMMAND_SET_OUTPUT_GAIN:
			kest_string_appendf(str, "(dB = %s%.04f)", cmd.val < 0 ? "" : " ", cmd.val);
			break;
			
		case COMMAND_ALLOC_FILTER:
			kest_string_appendf(str, "(order_ff = %d, order_fb = %d, format = %d)", cmd.data_1.order_ff, cmd.data_2.order_fb, cmd.format);
			break;
			
		case COMMAND_WRITE_FILTER_COEF:
		case COMMAND_UPDATE_FILTER_COEF:
			kest_string_appendf(str, "(handle = %d, coef = %d, val = %s%.04f)", cmd.data_1.handle,
				cmd.data_2.coef, cmd.val < 0 ? "" : " ", cmd.val);
			break;
			
		case COMMAND_COMMIT_FILTER_COEF:
			kest_string_appendf(str, "(handle = %d)", cmd.data_1.handle);
			break;
			
		default:
			return ERR_BAD_ARGS;
			break;
	}
	
	return NO_ERROR;
}
