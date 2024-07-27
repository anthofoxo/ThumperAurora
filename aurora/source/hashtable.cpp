#include "hashtable.hpp"

char const* aurora::lookupHash(uint32_t hash) {
	switch (hash) {
	// --- undocumented ---
	case 0x63259f0a: return "AnimComp";
	case 0x3c8efb12: return "EditStateComp";
	case 0x84e761eb: return "XfmComp";

	// --- object parameters ---

	// .lvl objects
	case 0xe04f3144: return "layer_volume";
	// .leaf objects
	case 0x90db6c5c: return "pitch";
	case 0x461da26a: return "roll";
	case 0x659c0bb8: return "turn";
	case 0x1e470b5d: return "turn_auto";
	case 0xb44b9ff1: return "scale_x";
	case 0x5c01c2b9: return "scale_y";
	case 0x2794bcce: return "scale_z";
	case 0x2e08b151: return "offset_x";
	case 0xfa4ab34f: return "offset_y";
	case 0x1ff804bc: return "offset_z";
	case 0x48697144: return "visibla01";
	case 0x842a58fa: return "visibla02";
	case 0xa444acf9: return "visible";
	case 0x93dcfe04: return "visiblz01";
	case 0x7ce4037d: return "visiblz02";
	// avatar .objlib objects
	case 0x7ec6de5f: return "sequin_speed";
	case 0x45e125b3: return "win";
	case 0xf31f3243: return "win_checkpoint";
	case 0xd37839a4: return "win_checkpoint_silent";
	// .samp objects
	case 0x82bb9c74: return "play";
	case 0x7a2ec56d: return "play_clean";
	case 0xa21622fa: return "pause";
	case 0x0bb8ea4b: return "resume";
	case 0x9fa68571: return "stop";
	// .mat objects
	case 0x13bdaa3b: return "emissive_color";
	case 0x7d06b11c: return "ambient_color";
	case 0x3dfe8a74: return "diffuse_color";
	case 0xd7b602ce: return "specular_color";
	case 0x1ca08372: return "reflectivity_color";
	case 0x52a06bf4: return "alpha";
	// .anim objects
	case 0xb3b5d9f1: return "frame";

	// --- interactive player objects ---

	// .spn objects: decorators/thump_rails.objlib
	case 0x52d7447a: return "thump_rails.a01";
	case 0x583cc8c2: return "thump_rails.a02";
	case 0x4477bbd4: return "thump_rails.ent";
	case 0xdc12888d: return "thump_rails.z01";
	case 0xbbf22741: return "thump_rails.z02";
	case 0x897c516b: return "thump_checkpoint.ent";
	case 0xadfa9f2c: return "thump_rails_fast_activat.ent";
	case 0x4733a5e6: return "thump_boss_bonus.ent";
	// .spn objects: decorators/thump_grindable.objlib
	case 0x51b93243: return "grindable_still.ent";
	case 0xcf53c501: return "left_multi.a01";
	case 0xbecb71fa: return "left_multi.a02";
	case 0x6e5b2576: return "left_multi.ent";
	case 0xd528868a: return "left_multi.z01";
	case 0xb9e107fc: return "center_multi.a02";
	case 0xed5a7e91: return "center_multi.ent";
	case 0xe27b6c3c: return "center_multi.z01";
	case 0x99f4653e: return "right_multi.a02";
	case 0x1e0ae961: return "right_multi.ent";
	case 0x03fd4938: return "right_multi.z01";
	case 0x73b0f245: return "right_multi.z02";
	// .spn objects : decorators / thump_grindable_multi.objlib
	case 0x3e774af9: return "grindable_quarters.ent";
	case 0x73af2bae: return "grindable_double.ent";
	case 0x2216f6df: return "grindable_thirds.ent";
	case 0x8b3738a4: return "grindable_with_thump.ent";
	// 	.spn objects: decorators/ducker.objlib
	case 0xc933e313: return "ducker_crak.ent";
	// .spn objects : decorators / jumper / jumper_set.objlib
	case 0x0743418a: return "jumper_1_step.ent";
	case 0x2eb30851: return "jumper_boss.ent";
	case 0x8d55abe4: return "jumper_6_step.ent";
	// .spn objects: decorators/jump_high/jump_high_set.objlib
	case 0xb886b204: return "jump_high.ent";
	case 0x9b004f93: return "jump_high_2.ent";
	case 0xcdfddfc4: return "jump_high_4.ent";
	case 0x10be3b0b: return "jump_high_6.ent";
	case 0x0ef04fb0: return "jump_boss.ent";
	// .spn objects : decorators / obstacles / wurms / millipede_half.objlib
	case 0x601c9d22: return "swerve_off.a01";
	case 0xd881767c: return "swerve_off.a02";
	case 0x3f602426: return "swerve_off.ent";
	case 0xc92199c0: return "swerve_off.z01";
	case 0x0d0004b1: return "swerve_off.z02";
	case 0x43eae5dd: return "millipede_half.a01";
	case 0xce42c1bf: return "millipede_half.a02";
	case 0xbe018127: return "millipede_half.ent";
	case 0xb01d4126: return "millipede_half.z01";
	case 0x45b829c9: return "millipede_half.z02";
	case 0xbf2a2b2a: return "millipede_half_phrase.a01";
	case 0xcacf6bea: return "millipede_half_phrase.a02";
	case 0xbbaf4e32: return "millipede_half_phrase.ent";
	case 0xf53455f0: return "millipede_half_phrase.z01";
	case 0xf28e0216: return "millipede_half_phrase.z02";
	// .spn objects: decorators/obstacles/wurms/millipede_quarter.objlib
	case 0x366799e4: return "millipede_quarter.a01";
	case 0x40b25fd4: return "millipede_quarter.a02";
	case 0x1703ae12: return "millipede_quarter.ent";
	case 0x66ac1596: return "millipede_quarter.z01";
	case 0xdb0dba2f: return "millipede_quarter.z02";
	case 0x041539b3: return "millipede_quarter_phrase.a01";
	case 0x4c1c613a: return "millipede_quarter_phrase.a02";
	case 0x1d2ce5f4: return "millipede_quarter_phrase.ent";
	case 0x0bc1ab97: return "millipede_quarter_phrase.z01";
	case 0x1e48e54e: return "millipede_quarter_phrase.z02";
	// .spn objects: decorators/sentry.objlib
	case 0xf1e67274: return "sentry.ent";
	case 0x8792db5a: return "level_9.ent";
	case 0x32eb179b: return "level_5.ent";
	case 0x476dfd28: return "level_8.ent";
	case 0x32e9a51e: return "sentry_boss.ent";
	case 0xbfeb686e: return "level_7.ent";
	case 0xac58b0b9: return "level_6.ent";
	case 0xc70f3afd: return "sentry_boss_multilane.ent";
	case 0xb4034d58: return "level_8_multi.ent";
	case 0xb87c60e8: return "level_9_multi.ent";

	// --- decorative objects ---

	// .spn objects: decorators/jump_high/jump_high_big_trees_set.objlib
	case 0xa8d60562: return "trees.ent";
	case 0xbb63511d: return "trees_16.ent";
	case 0xc5af922f: return "trees_4.ent";
	// .spn objects: entity/ambient_fx.objlib
	case 0x1e7bd174: return "speed_streaks_short.ent";
	case 0x87cdcba8: return "speed_streaks_RGB.ent";
	case 0xc66f96dd: return "smoke.ent";
	case 0xd7843636: return "death_shatter.ent";
	case 0x55a80a99: return "speed_streaks.ent";
	case 0x76414bb7: return "data_streaks_radial.ent";
	case 0x49113923: return "boss_7_tunnel_enter.ent";
	case 0xdb619bed: return "boss_damage_stage4.ent";
	case 0xa28811db: return "crakhed_damage.ent";
	case 0x6f02512b: return "win_debris.ent";
	case 0xa8bd36c8: return "crakhed_destroy.ent";
	case 0x71dc045d: return "stalactites.ent";
	case 0x6ac944d0: return "aurora.ent";
	case 0x01a07bd9: return "vortex_decorator.ent";
	case 0x718ff746: return "boss_damage_stage3.ent";
	case 0xd8258d1a: return "boss_damage_stage1.ent";
	case 0xb00a87d0: return "boss_damage_stage2.ent";
	// skybox_colors.flow (levels/demo.objlib)
	case 0xd8e402da: return "black";
	case 0x1411f1a3: return "crakhed";
	case 0x4bb91040: return "dark_blue";
	case 0xfa50813b: return "dark_green";
	case 0x30656178: return "dark_red";
	case 0xc0c8325a: return "light_blue";
	case 0x3b652d19: return "light_green";
	case 0xa7aa53f0: return "light_red";

	// --- SFX objects ---

	// 	turn_anticipation.flow (levels/Level6/level_6.objlib)
	case 0xe5745bf3: return "fire";
	// dissonant_bursts.flow (global/dissonant_bursts.objlib)
	case 0xe073575f: return "diss11";
	// french_horn_chords.flow (global/french_horn_chords.objlib)
	case 0x79603dd3: return "french12";

	// --- Bosses (.gate objects only) ---

	// 	.spn objects: boss/gate_triangle/triangle_boss.objlib
	case 0x8c0dd639: return "tutorial_thumps.ent";
	// .spn objects: boss/boss_spiral/gate_spiral.objlib
	case 0xc7c056e2: return "boss_gate_pellet.ent";
	default: return nullptr;
	}
}