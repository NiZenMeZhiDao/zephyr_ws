#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/chassis.h>

#define DT_DRV_COMPAT ares_swchassis

#define CHASSIS_STACK_SIZE 2048

#define CHASSIS_DATA_INST(inst)                                                                    \
	static chassis_data_t chassis_data_##inst = {                                              \
		.chassis_status = {0},                                                             \
		.targetYaw = 0.0f,                                                                 \
		.currentYaw = 0.0f,                                                                \
		.targetGyro = 0.0f,                                                                \
		.currentGyro = 0.0f,                                                               \
		.targetXSpeed = 0.0f,                                                              \
		.targetYSpeed = 0.0f,                                                              \
		.currentXSpeed = 0.0f,                                                             \
		.currentYSpeed = 0.0f,                                                             \
		.currTime = 0ULL,                                                                  \
		.prevTime = 0ULL,                                                                  \
		.angleControl = true,                                                              \
		.angle_to_center = {0.0f},                                                         \
		.distance_to_center = {0.0f},                                                      \
		.static0 = 0,                                                                      \
		.pid_input = 0,                                                                    \
	};

#define STEERWHEELS_FOREACH(inst, fn)                                                              \
	{DT_FOREACH_PROP_ELEM_SEP(DT_DRV_INST(inst), steerwheels, fn, (,))}

#define GET_SW_DEVICE(node_id, prop, idx) DEVICE_DT_GET(DT_PHANDLE_BY_IDX(node_id, prop, idx))

#define GET_SW_X_OFFSET(node_id, prop, idx)                                                        \
	(float)((int)DT_PHA_BY_IDX(node_id, prop, idx, offset_x)) / 10000.0f

#define GET_SW_Y_OFFSET(node_id, prop, idx)                                                        \
	(float)((int)DT_PHA_BY_IDX(node_id, prop, idx, offset_y)) / 10000.0f

#define CHASSIS_CONFIG_INST(inst)                                                                  \
	static const chassis_cfg_t chassis_cfg_##inst = {                                          \
		.angle_pid = DEVICE_DT_GET(DT_PROP(DT_DRV_INST(inst), angle_pid)),                 \
		.steerwheels = STEERWHEELS_FOREACH(inst, GET_SW_DEVICE),                           \
		.pos_X_offset = STEERWHEELS_FOREACH(inst, GET_SW_X_OFFSET),                        \
		.pos_Y_offset = STEERWHEELS_FOREACH(inst, GET_SW_Y_OFFSET),                        \
	};

#define CHASSIS_DEFINE_INST(inst)                                                                  \
	DEVICE_DT_DEFINE(DT_DRV_INST(inst), swchassis_init, NULL, &chassis_data_##inst,            \
			 &chassis_cfg_##inst, POST_KERNEL, CONFIG_CHASSIS_INIT_PRIORITY,           \
			 &swchassis_driver_api);

#define CHASSIS_INIT(inst)                                                                         \
	CHASSIS_CONFIG_INST(inst)                                                                  \
	CHASSIS_DATA_INST(inst)                                                                    \
	CHASSIS_DEFINE_INST(inst)
