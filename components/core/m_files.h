#ifndef M_FILES_H_
#define M_FILES_H_

#define M_PROFILE_MAGIC_BYTE  			0x4a
#define M_PROFILE_PIPELINE_LINEAR  		0x01

#define M_PROFILE_BROKEN_TRANSFORMER	0xfffd

#define M_STATE_MAGIC_BYTE  			0x4b

#define M_SEQUENCE_MAGIC_BYTE  			0x4c

#define M_WRITE_UNFINISHED_BYTE  		0xfe
#define M_WRITE_FINISHED_BYTE  			0xff

#ifdef M_DESKTOP
#define MAIN_SEQUENCE_FNAME "./sdcard/ms.seq"
#define SETTINGS_FNAME 		"./sdcard/conf"
#define M_PROFILES_DIR  	"./sdcard/prof/"
#define M_SEQUENCES_DIR 	"./sdcard/seq/"
#define M_EFFECT_DESC_DIR 	"./sdcard/eff/"
#else
#define MAIN_SEQUENCE_FNAME "/sdcard/ms.seq"
#define SETTINGS_FNAME 		"/sdcard/conf"
#define M_PROFILES_DIR  	"/sdcard/prof/"
#define M_SEQUENCES_DIR 	"/sdcard/seq/"
#define M_EFFECT_DESC_DIR 	"/sdcard/eff/"
#endif


#define PROFILE_EXTENSION  ".prf"
#define SEQUENCE_EXTENSION ".seq"
#define EFF_DESC_EXTENSION ".eff"

int save_profile_as_file		(m_profile *profile, const char *fname);
int save_profile_as_file_safe	(m_profile *profile, const char *fname);
int read_profile_from_file		(m_profile *profile, const char *fname);

int save_sequence_as_file		(m_sequence *sequence, const char *fname);
int save_sequence_as_file_safe	(m_sequence *sequence, const char *fname);
int read_sequence_from_file		(m_sequence *sequence, const char *fname);

struct m_state;
int save_state_to_file(struct m_state *state, const char *fname);
int load_state_from_file(struct m_state *state, const char *fname);

int load_effects(m_context *cxt);

int init_periodic_backup_task();

int save_profile(m_profile *profile);
int load_saved_profiles(m_context *cxt);

int save_sequence(m_sequence *sequence);
int load_saved_sequences(m_context *cxt);

int safe_file_write(int (*write_func)(void *arg, const char *fname), void *arg, const char *fname);

string_ll *list_files_in_directory(char *dir);

int m_init_directories();

void erase_sd_card_void_cb(void *data);
void erase_sd_card();

int fnames_agree(char *a, char *b);

#endif
