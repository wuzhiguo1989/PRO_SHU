# include "config.h"
//超时记录标志位
extern volatile bool  candidate_time_out_signal_recieving;
//开柜门驱动时间标志位
extern volatile bool  candidate_driver_signal_recieving;
//开柜门锁编号
extern volatile u8 key_cabinet_number;