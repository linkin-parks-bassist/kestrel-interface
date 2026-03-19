#ifndef KEST_INT_PIPELINE_H_
#define KEST_INT_PIPELINE_H_


typedef struct
{
	kest_effect_pll *effects;
} kest_pipeline;

int init_m_pipeline(kest_pipeline *pipeline);

kest_effect *kest_pipeline_append_effect_eff(kest_pipeline *pipeline, kest_effect_desc *eff);

int kest_pipeline_move_effect(kest_pipeline *pipeline, int new_pos, int old_pos);
int kest_pipeline_remove_effect(kest_pipeline *pipeline, uint16_t id);
int kest_pipeline_get_n_effects(kest_pipeline *pipeline);

kest_effect *kest_pipeline_get_effect_by_id(kest_pipeline *pipeline, int id);

int clone_pipeline(kest_pipeline *dest, kest_pipeline *src);
void gut_pipeline(kest_pipeline *pipeline);

int kest_pipeline_create_fpga_transfer_batch(kest_pipeline *pipeline, kest_fpga_transfer_batch *batch);

#endif
