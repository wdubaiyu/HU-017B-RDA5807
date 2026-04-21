#ifndef _CONFIG_5807M_
#define _CONFIG_5807M_


sbit W1 = P1 ^ 0;
sbit W2 = P1 ^ 1;
sbit W3 = P1 ^ 6;
sbit W4 = P1 ^ 7;

// 自定义全局类型 uint8_t 一个字节
typedef unsigned char uint8_t;
// 自定义全局类型 uint16_t 两个字节 高位在前，低位在后
typedef unsigned short int uint16_t;

// 全局变量
extern uint8_t sys_band;
extern uint8_t sys_vol;
extern uint16_t sys_freq;
extern uint8_t sys_radio_index;
extern uint8_t sys_radio_index_max;
extern bit sys_sleep_mode;
// 轮询显示SNR和RSSI
extern bit cycle_in_freq_rssi;


/**
 * @brief 复位配置
 * 
 * 复位所有配置到默认值
 */
void CONF_RESET(void);

/**
 * 开机初始化读取配置
 */
uint8_t CONF_SYS_INIT(void);

/**
 * @brief 读取最小间隔
 * Channel Spacing. 
    0 = 100 kHz 
    1 = 200 kHz 
    2 = 50kHz 
    3 = 25KHz 
 * @return uint8_t 
 */
uint8_t CONF_READ_SPACE(uint8_t band_sel);

/**
 * @brief 读取band
    Band Select. 
    0 = 87–108 MHz (US/Europe)
    1 = 76–91 MHz (Japan) 
    2 = 76–108 MHz (world wide) 
    3 = 65 –76 MHz（If 0x07h_bit<9> ( band )=1, 65-76MHz; =0, 50-76MHz）
    4 = 50-65MHz （If 0x07h_bit<9> ( band )=1, 65-76MHz; =0, 50-76MHz） 0100=4
 * 
 * @return uint8_t 
 */
uint8_t CONF_READ_BAND(void);


/**
 * 触发写配置
 */
void CONF_WRITE(void);


/**
 * @brief 通过索引获取电台频率
 * 
 * @param index 0~254
 * @return uint16_t 频率值
 */
uint16_t CONF_GET_FREQ_BY_INDEX(uint8_t index);


/**
 * @brief 擦除电台eeprom
 * 
 * @param band 频段 0~4 
 */
void CONF_RADIO_ERASE();

/**
 * 追加一个电台
 */
void CONF_RADIO_PUT(uint8_t index, uint16_t freq);

/**
 * 搜台完成,保存频道总数
 */
void CONF_WRITE_INDEX_MAX(uint8_t index);

#endif
