#ifndef KEST_UPDATE_H_
#define KEST_UPDATE_H_

#define KEST_UPDATE_NONE 	0
#define KEST_UPDATE_PARAM	1
#define KEST_UPDATE_PRESET	2
#define KEST_UPDATE_MEM		3

typedef struct {
	int type;
	
	union {
		kest_parameter *param;
		kest_preset *preset;
	} data;
} kest_update;

#define KEST_UPDATER_STATE_READY 		0
#define KEST_UPDATER_STATE_REPROGRAM 	1

#define KEST_BLOCK_INSTR_WRITE 	1
#define KEST_BLOCK_REG_WRITE 	2
#define KEST_BLOCK_REG_UPDATE 	3
#define KEST_FILTER_COEF_WRITE 	4
#define KEST_FILTER_COEF_UPDATE 5

typedef struct {
	int type;
	int addr_1;
	int addr_2;
	int format;
	uint32_t instr;
	kest_scope *scope;
	kest_expression *expr;
} kest_fpga_write;

#define KEST_ALLOC_TYPE_FILTER  1
#define KEST_ALLOC_TYPE_DELAY	2

typedef struct {
	int type;
	int size_1;
	int size_2;
	int format;
} kest_fpga_alloc;


typedef struct {
	int addr;
	uint64_t period_ms;
	uint64_t last_t;
	
	kest_fpga_read_spec read;
} kest_fpga_mem_read;

DECLARE_LIST(kest_fpga_write);
DECLARE_LIST(kest_fpga_alloc);
DECLARE_LIST(kest_fpga_mem_read);

typedef struct {
	int state;
	
	kest_preset *active_preset;
	
	kest_fpga_alloc_list allocs;
	
	kest_fpga_write_list instr_writes;
	kest_fpga_write_list reg_writes;
	kest_fpga_write_list filter_writes;
	
	kest_fpga_command_list cmds;
	kest_fpga_transfer_batch batch;
	
	kest_fpga_mem_read_list reads;
	
	int tick_ctr;
	
} kest_updater_state;

int kest_update_task_start();

int kest_update_queue(kest_update update);

int kest_updater_notify_param(kest_parameter *param);
int kest_updater_notify_preset(kest_preset *preset);

void kest_active_preset_updater_start();

int kest_active_preset_updater_notify_effect_by_ptr(kest_effect *effect);
int kest_active_preset_updater_notify_effect_by_id(int preset_id, int effect_id);
int kest_active_preset_updater_notify_param (int preset_id, int effect_id, int parameter_id);

kest_fpga_transfer_batch kest_standalone_generate_program_batch(kest_effect_ptr_list *effects);

#endif
