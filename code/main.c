#include <STC8G.H>
#include "delay.h"
#include "config/Config.h"
#include "config/EEPROM.h"
#include "rda5807/i2c.h"
#include "rda5807/RDA5807M.h"
#include "led/myLed.h"
#include "key/key.h"
// #include "uart/uart.h"
#include "time/timer.h"

// （自动搜台设置==11）
// （定时关机设置==21）
uint8t key_function_flag;

// 电源状态（0正常，1关机倒计时，2已关机）
uint8t POWER_STATUS = 0x00;
uint16t timed_stanby_count;

// 需要系统写
bit conf_write_flag;
bit rssi_read_flag;

void triggerWriteFreq()
{
	sys_write_freq_flag = 1;
	CONF_WRITE();
	conf_write_flag = 0;
}

// 按键触发功能
void userInput(uint8t Key_num)
{
	// 有按键操作时重置睡眠时间
	LED_RESET_SLEEP_TIME();

	if (key_function_flag > 10) // 功能菜单触发后不响应基础操作
	{
		// 触发功能后按了V+
		if (Key_num == 1)
		{
			if (key_function_flag == 11)
			{
				LED_SNR = RDA5807M_Read_SNR();
				if (LED_SNR == 15)
				{
					LED_SNR = 0;
				}
				else
				{
					LED_SNR++;
				}
				RDA5807M_Set_SNR(LED_SNR);
			}

			if (key_function_flag == 21)
			{
				LED_TIMED_STANDBY_U();
			}

			return;
		}

		// 触发功能后按了V-
		if (key_function_flag > 10 && Key_num == 2)
		{
			if (key_function_flag == 11)
			{
				LED_SNR = RDA5807M_Read_SNR();
				if (LED_SNR == 0)
				{
					LED_SNR = 15;
				}
				else
				{
					LED_SNR--;
				}
				RDA5807M_Set_SNR(LED_SNR);
			}

			if (key_function_flag == 21)
			{
				LED_TIMED_STANDBY_D();
			}

			return;
		}

		// 触发功能后按了F+  (取消)
		if (key_function_flag > 10 && Key_num == 3)
		{
			if (key_function_flag == 11)
			{
			}

			if (key_function_flag == 21)
			{
				POWER_STATUS = 0;
			}

			// 取消功能直接展示频率
			LED_SET_DISPLY_TYPE(0);
			key_function_flag = 0;
			return;
		}

		// 触发功能后按了F- （确认）
		if (key_function_flag > 10 && Key_num == 4)
		{
			if (key_function_flag == 11)
			{
				RDA5807M_Search_Automatic();
			}

			if (key_function_flag == 21)
			{
				// 定时关机大于0才能确认
				if (LED_TIMED_STANDBY > 0)
				{
					POWER_STATUS = 1;
					timed_stanby_count = 0;
					Timer1_Init(); // 定时器开始计时
					LED_SET_DISPLY_TYPE(1);
				}
				else
				{
					POWER_STATUS = 0;

					LED_SET_DISPLY_TYPE(0);
				}
			}

			key_function_flag = 0;
			return;
		}

		// 如果触发了功能则不进行下面的 操作
		return;
	}

	///////////////////////////////////////////////////////////////功能出发区域///////////////////
	// K12 设置省电模式（一定时间后关闭数码管）
	if (Key_num == 12)
	{
		LED_CHANGE_SLEEP_MODE();
		return;
	}

	// K13 自动搜台 触发
	if (Key_num == 13)
	{
		LED_SNR = RDA5807M_Read_SNR();
		key_function_flag = 11;
		LED_SET_DISPLY_TYPE(101); // 显示snr设置
		return;
	}

	// K11 设置定时关机
	if (Key_num == 11)
	{
		key_function_flag = 21;
		// 设置定时关机时，取消上一次设定
		POWER_STATUS = 0;
		Timer1_Stop(); // 定时器停止计时
		LED_SET_DISPLY_TYPE(102);
		return;
	}

	// K14 切换POLL显示
	if (Key_num == 14)
	{
		cycle_in_freq_rssi = ~cycle_in_freq_rssi;
		sys_write_poll_flag = 1;
		LED_SET_DISPLY_TYPE(5);
		return;
	}

	///////////////////////////////////////////////////////////////功能出发区域///////////////////

	//////////////////////////////////////////////////////////////常用操作区域
	// K33 手动搜下一个台
	if (Key_num == 33)
	{
		sys_freq = RDA5807M_Seek(1);
		return;
	}

	// K44 手动搜上一个台
	if (Key_num == 44)
	{
		sys_freq = RDA5807M_Seek(0);
		return;
	}

	// K22 V-长按
	if (Key_num == 22)
	{
		LED_SET_DISPLY_TYPE(4); // 数码管显示音量
		RDA5807M_CHANGE_MUTE();
		return;
	}

	// V+
	if (Key_num == 1)
	{
		LED_SET_DISPLY_TYPE(4); // 数码管显示音量
		// 最大音量15
		if (sys_vol + 1 < 16)
		{
			RDA5807M_Set_Volume(sys_vol + 1);
		}
		return;
	}
	// V-
	if (Key_num == 2)
	{
		LED_SET_DISPLY_TYPE(4); // 数码管显示音量
		// 最小音量1
		if (sys_vol > 0)
		{
			RDA5807M_Set_Volume(sys_vol - 1);
		}
		return;
	}
	// F+
	if (Key_num == 3)
	{

		if (sys_radio_index == sys_radio_index_max)
		{
			sys_radio_index = 0;
		}
		else
		{
			++sys_radio_index;
		}
		LED_FRE_REAL = CONF_GET_RADIO_INDEX(sys_radio_index);
		RDA5807M_Set_Freq(LED_FRE_REAL);
		triggerWriteFreq();
		return;
	}
	// F-
	if (Key_num == 4)
	{

		if (sys_radio_index == 0)
		{
			sys_radio_index = sys_radio_index_max;
		}
		else
		{
			--sys_radio_index;
		}

		LED_FRE_REAL = CONF_GET_RADIO_INDEX(sys_radio_index);
		RDA5807M_Set_Freq(LED_FRE_REAL);
		// 需要写立即写配置
		triggerWriteFreq();
		return;
	}
}

/**
 * 启动收音函数
 */
void InitSystem()
{
	bit autoMatic = 0;
	POWER_STATUS = 0x00;

	Timer1_Stop();
	// 读取系统持久化配置，返回是否需要自动搜台
	autoMatic = CONF_SYS_INIT();
	key_function_flag = 0x00;
	LED_TIMED_STANDBY = 0x1E;

	//打开数码管显示和按键扫描
	Timer0Init();
	EA = 1;

	// 初始化收音机
	Delay(300); // 等待收音机芯片上的稳定后再初始化
	RDA5807M_init();

	LED_RESET_SLEEP_TIME();
	if (autoMatic) // 加载上一次系统配置,返回是否需要自动搜台
	{
		RDA5807M_Search_Automatic();
	}
	else
	{ // 播放上次关机时的电台
		LED_FRE_REAL = sys_freq;
		LED_SET_DISPLY_TYPE(10);
	}
}

void main()
{
	uint8t Key_num;
	I2C_Init();
	// UartInit();
	// printf("UartInit...");
	InitSystem();

	// printf("main...");
	while (1)
	{

		// 是否切换到显示RSSI
		if (rssi_read_flag)
		{
			LED_RSSI = RDA5807M_Read_RSSI();
			LED_SET_DISPLY_TYPE(2);
			LED_DISPLAY_REC_COUNT = LED_REC_TIME / 2;
			rssi_read_flag = 0; // 重置标记
		}

		// 读取用户按键输入
		Key_num = POP_KEY();
		// 在关机中
		if (POWER_STATUS == 2)
		{
			// 关机中长按F-开机
			if (Key_num == 44)
			{
				InitSystem();
			}

			continue;
		}

		// 关机时间到
		if (POWER_STATUS == 1 && LED_TIMED_STANDBY < 1)
		{
			POWER_STATUS = 2;
			Timer1_Stop(); // 定时器停止计时
			RDA5807M_OFF();
			W1 = W2 = W3 = W4 = 0; // 关闭数码管
			continue;
		}

		// 按键有效,响应用户操作
		if (Key_num)
		{
			// printf("Key_num.. %d \n", (int)Key_num);
			userInput(Key_num);
		}

		// 主程序持久化触发（保存播放频率、音量、睡眠模式）
		if (conf_write_flag)
		{
			CONF_WRITE();
			conf_write_flag = 0;
		}
	}
}

void Timer0_Isr(void) interrupt 1
{

	uint8t led_type = LED_GET_DISPLY_TYPE();

	// 轮询按键
	Key_Loop();

	// {				// 调试用关闭数码详细（与串口公用IO口用冲突，调试时不显示）
	// 	TL0 = 0x66; // 设置定时初始值
	// 	TH0 = 0xFC; // 设置定时初始值
	// 	TF0 = 0;	// 清除TF0标志,进入下一次计时
	// 	return;
	// }

	// 不是关机状态才显示数码管
	if (POWER_STATUS < 2)
	{
		// 数码管关闭不执行和数码管相关的操作
		if (Led_Loop())
		{
			TL0 = 0x66; // 设置定时初始值
			TH0 = 0xFC; // 设置定时初始值
			TF0 = 0;	// 清除TF0标志,进入下一次计时
			return;
		}

		// 数码管处于显示中，需要切换为freq显示
		if (led_type != 10) // 10是频率显示
		{
			if (led_type < 100) // 是否需要显示恢复为频率
			{
				if (++LED_DISPLAY_REC_COUNT >= LED_REC_TIME)
				{
					LED_SET_DISPLY_TYPE(10);
					// 数码管恢复显示时触发持久化操作（不包括FREQ——index）
					conf_write_flag = 1;
				}
			}
		}
		else if (cycle_in_freq_rssi) // 显示的是10频率，开启了rssi轮询显示
		{
			if (++LED_DISPLAY_REC_COUNT >= LED_REC_TIME)
			{
				// 打开读取rssi功能
				rssi_read_flag = 1;
			}
		}
	}

	TL0 = 0x66; // 设置定时初始值
	TH0 = 0xFC; // 设置定时初始值
	TF0 = 0;	// 清除TF0标志,进入下一次计时
}

// 定时关机功能开启,循环减时间
void Timer1_Isr(void) interrupt 3
{
	if (POWER_STATUS == 1)
	{
		if (++timed_stanby_count >= 3000)
		{
			LED_TIMED_STANDBY -= 1; // 减去一分钟
			timed_stanby_count = 0; // 重新计数
		}
	}
}