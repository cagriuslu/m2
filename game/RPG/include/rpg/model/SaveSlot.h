#ifndef IMPL_SAVESLOT_H
#define IMPL_SAVESLOT_H

//begin_serializable(PlayerSaveData);
//	serializable_long_array(power_up_indexes);
//
//	[[nodiscard]] bool validate() const;
//end_serializable();
//
//begin_serializable(ChapterSaveData);
//	serializable_long_array(stage_levels);
//	serializable_double_array(min_completion_times_s);
//
//	[[nodiscard]] bool validate() const;
//end_serializable();
//
//begin_serializable(SaveData);
//	serializable_long(points);
//	serializable_struct(PlayerSaveData, player);
//	serializable_struct_array(ChapterSaveData, chapters);
//
//	[[nodiscard]] bool validate() const;
//end_serializable();

#endif //IMPL_SAVESLOT_H
