#ifndef KEST_FILES_H_
#define KEST_FILES_H_

#define KEST_PRESET_MAGIC_BYTE  			0x4a
#define KEST_PRESET_PIPELINE_LINEAR  		0x01

#define KEST_PRESET_BROKEN_EFFECT			0xfffd

#define KEST_STATE_MAGIC_BYTE  				0x4b

#define KEST_SEQUENCE_MAGIC_BYTE  			0x4c

#define KEST_WRITE_UNFINISHED_BYTE  		0xfe
#define KEST_WRITE_FINISHED_BYTE  			0xff

#ifdef KEST_DESKTOP
#define MAIN_SEQUENCE_FNAME 	"./sdcard/ms.seq"
#define SETTINGS_FNAME 			"./sdcard/conf"
#define KEST_PRESETS_DIR  		"./sdcard/pre/"
#define KEST_SEQUENCES_DIR 		"./sdcard/seq/"
#define KEST_EFFECT_DESC_DIR 	"./sdcard/eff/"
#else
#define MAIN_SEQUENCE_FNAME 	"/sdcard/ms.seq"
#define SETTINGS_FNAME 			"/sdcard/conf"
#define KEST_PRESETS_DIR  		"/sdcard/pre/"
#define KEST_SEQUENCES_DIR 		"/sdcard/seq/"
#define KEST_EFFECT_DESC_DIR 	"/sdcard/eff/"
#endif


#define PRESET_EXTENSION   ".pre"
#define SEQUENCE_EXTENSION ".seq"
#define EFF_DESC_EXTENSION ".eff"

int save_preset_as_file		(kest_preset *preset, const char *fname);
int save_preset_as_file_safe(kest_preset *preset, const char *fname);
int read_preset_from_file	(kest_preset *preset, const char *fname);

int save_sequence_as_file		(kest_sequence *sequence, const char *fname);
int save_sequence_as_file_safe	(kest_sequence *sequence, const char *fname);
int read_sequence_from_file		(kest_sequence *sequence, const char *fname);

struct kest_state;
int save_state_to_file(struct kest_state *state, const char *fname);
int load_state_from_file(struct kest_state *state, const char *fname);

int load_effects(kest_context *cxt);

int init_periodic_backup_task();

int save_preset(kest_preset *preset);
int load_saved_presets(kest_context *cxt);

int save_sequence(kest_sequence *sequence);
int load_saved_sequences(kest_context *cxt);

int safe_file_write(int (*write_func)(void *arg, const char *fname), void *arg, const char *fname);

string_ll *list_files_in_directory(char *dir);

int kest_init_directories();

void erase_sd_card_void_cb(void *data);
void erase_sd_card();

int fnames_agree(char *a, char *b);

#endif
