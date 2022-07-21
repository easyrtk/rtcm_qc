#ifndef _HY_RTCM_BUFF_H
#define _HY_RTCM_BUFF_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RTCM_BUF_LEN 1200

typedef struct {        /* RTCM control struct type */
    int staid;          /* station id */
    int type;
    int nbyte;          /* number of bytes in message buffer */ 
    int nbit;           /* number of bits in word buffer */ 
    int len;            /* message length (bytes) */
    unsigned char nsat; /* number of satellites */
    unsigned char nsig; /* number of signals */
    unsigned char ncel; /* number of cell */
    unsigned char sats[64];         /* satellites */
    unsigned char sigs[32];         /* signals */
    unsigned char cels[64];     /* cell mask */
    unsigned char buff[MAX_RTCM_BUF_LEN]; /* message buffer */
    unsigned short wk;
    double tow;
    double pos[3];
	int sync;
    int crc;
    int slen;
    int mark;
    unsigned char sys;         /* update satellite of ephemeris */
    unsigned char prn;
} rtcm_buff_t;

int input_rtcm3_type(rtcm_buff_t* rtcm, unsigned char data);

int decode_type1005_(unsigned char* buff, int len, int* staid, double* pos);
int decode_type1006_(unsigned char* buff, int len, int* staid, double* pos);
int update_type_1005_1006_pos(uint8_t* buff, int nbyte, double* p);

//void setbitu(unsigned char* buff, int pos, int len, unsigned int data);
//unsigned int getbitu(const unsigned char* buff, int pos, int len);
//int getbits(const unsigned char *buff, int pos, int len);

//unsigned int crc24q(const unsigned char* buff, int len);

int rtcm_obs_type(int type);
int rtcm_eph_type(int type);

#ifdef __cplusplus
}
#endif

#endif
