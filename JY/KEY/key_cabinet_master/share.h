# include "config.h"
//超时记录标志位
extern volatile bool  candidate_time_out_signal_recieving;
//开柜门驱动时间标志位
extern volatile bool  candidate_driver_signal_recieving;
//开柜门锁编号
extern volatile u8 key_cabinet_number;
//柜门性能
//D7  D6  D5  D4  D3  D2  D1
//D7:220V电源检测（1：丢失、0：正常）（电池正常、220V为零）
//D6:电池电量（1：低、0：正常）（220V存在，电池电量低）
//D5:柜门锁（1：有问题、0：正常）
//其余无定义，默认为零
extern volatile u8 performance_status;
// 记录问题柜门编号
extern volatile u8 order_for_badkey[30];
// 记录问题柜门数目
extern volatile u8 number_for_badkey;

