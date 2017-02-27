#define ERROR_CODE_TRUE         0
#define ERROR_CODE_FALSE        1
#define ERROR_CODE_WRITE_ADDR   10
#define ERROR_CODE_WRITE_DATA   20
#define ERROR_CODE_READ_ADDR    30
#define ERROR_CODE_READ_DATA    40
#define ERROR_CODE_COMPARE      50

#define DEV_ADDR			0x7a

#define CMD_BYPASS          0x03
#define CMD_RESET           0x04
#define CMD_MODE0           0x05
#define CMD_MODE1           0x06
#define CMD_KEYGEN          0x07
#define CMD_AES_ENC         0x08
#define CMD_AES_DEC         0x09
//#define CMD_RX_RAND         0x0A
#define CMD_TX_RAND         0x0E
#define CMD_USER_KEY_SET    0x11
//#define CMD_AES_OPTION      0x1A


/***********************************************************************************
 * Function Proto Type
 ***********************************************************************************/
void ENTANG_key_init();
unsigned char ENTANG_A_init(unsigned char *ENTANG_A_user_key_password, unsigned char *key_in);
unsigned char ENTANG_A_enc(unsigned char *pt, unsigned char *ct);
unsigned char ENTANG_A_dec(unsigned char *pt, unsigned char *ct);