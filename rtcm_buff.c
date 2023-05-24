#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "rtcm_buff.h"

#define RTCM3PREAMB 0xD3        /* rtcm ver.3 frame preamble */

static const unsigned int tbl_CRC24Q[]={
    0x000000,0x864CFB,0x8AD50D,0x0C99F6,0x93E6E1,0x15AA1A,0x1933EC,0x9F7F17,
    0xA18139,0x27CDC2,0x2B5434,0xAD18CF,0x3267D8,0xB42B23,0xB8B2D5,0x3EFE2E,
    0xC54E89,0x430272,0x4F9B84,0xC9D77F,0x56A868,0xD0E493,0xDC7D65,0x5A319E,
    0x64CFB0,0xE2834B,0xEE1ABD,0x685646,0xF72951,0x7165AA,0x7DFC5C,0xFBB0A7,
    0x0CD1E9,0x8A9D12,0x8604E4,0x00481F,0x9F3708,0x197BF3,0x15E205,0x93AEFE,
    0xAD50D0,0x2B1C2B,0x2785DD,0xA1C926,0x3EB631,0xB8FACA,0xB4633C,0x322FC7,
    0xC99F60,0x4FD39B,0x434A6D,0xC50696,0x5A7981,0xDC357A,0xD0AC8C,0x56E077,
    0x681E59,0xEE52A2,0xE2CB54,0x6487AF,0xFBF8B8,0x7DB443,0x712DB5,0xF7614E,
    0x19A3D2,0x9FEF29,0x9376DF,0x153A24,0x8A4533,0x0C09C8,0x00903E,0x86DCC5,
    0xB822EB,0x3E6E10,0x32F7E6,0xB4BB1D,0x2BC40A,0xAD88F1,0xA11107,0x275DFC,
    0xDCED5B,0x5AA1A0,0x563856,0xD074AD,0x4F0BBA,0xC94741,0xC5DEB7,0x43924C,
    0x7D6C62,0xFB2099,0xF7B96F,0x71F594,0xEE8A83,0x68C678,0x645F8E,0xE21375,
    0x15723B,0x933EC0,0x9FA736,0x19EBCD,0x8694DA,0x00D821,0x0C41D7,0x8A0D2C,
    0xB4F302,0x32BFF9,0x3E260F,0xB86AF4,0x2715E3,0xA15918,0xADC0EE,0x2B8C15,
    0xD03CB2,0x567049,0x5AE9BF,0xDCA544,0x43DA53,0xC596A8,0xC90F5E,0x4F43A5,
    0x71BD8B,0xF7F170,0xFB6886,0x7D247D,0xE25B6A,0x641791,0x688E67,0xEEC29C,
    0x3347A4,0xB50B5F,0xB992A9,0x3FDE52,0xA0A145,0x26EDBE,0x2A7448,0xAC38B3,
    0x92C69D,0x148A66,0x181390,0x9E5F6B,0x01207C,0x876C87,0x8BF571,0x0DB98A,
    0xF6092D,0x7045D6,0x7CDC20,0xFA90DB,0x65EFCC,0xE3A337,0xEF3AC1,0x69763A,
    0x578814,0xD1C4EF,0xDD5D19,0x5B11E2,0xC46EF5,0x42220E,0x4EBBF8,0xC8F703,
    0x3F964D,0xB9DAB6,0xB54340,0x330FBB,0xAC70AC,0x2A3C57,0x26A5A1,0xA0E95A,
    0x9E1774,0x185B8F,0x14C279,0x928E82,0x0DF195,0x8BBD6E,0x872498,0x016863,
    0xFAD8C4,0x7C943F,0x700DC9,0xF64132,0x693E25,0xEF72DE,0xE3EB28,0x65A7D3,
    0x5B59FD,0xDD1506,0xD18CF0,0x57C00B,0xC8BF1C,0x4EF3E7,0x426A11,0xC426EA,
    0x2AE476,0xACA88D,0xA0317B,0x267D80,0xB90297,0x3F4E6C,0x33D79A,0xB59B61,
    0x8B654F,0x0D29B4,0x01B042,0x87FCB9,0x1883AE,0x9ECF55,0x9256A3,0x141A58,
    0xEFAAFF,0x69E604,0x657FF2,0xE33309,0x7C4C1E,0xFA00E5,0xF69913,0x70D5E8,
    0x4E2BC6,0xC8673D,0xC4FECB,0x42B230,0xDDCD27,0x5B81DC,0x57182A,0xD154D1,
    0x26359F,0xA07964,0xACE092,0x2AAC69,0xB5D37E,0x339F85,0x3F0673,0xB94A88,
    0x87B4A6,0x01F85D,0x0D61AB,0x8B2D50,0x145247,0x921EBC,0x9E874A,0x18CBB1,
    0xE37B16,0x6537ED,0x69AE1B,0xEFE2E0,0x709DF7,0xF6D10C,0xFA48FA,0x7C0401,
    0x42FA2F,0xC4B6D4,0xC82F22,0x4E63D9,0xD11CCE,0x575035,0x5BC9C3,0xDD8538
};

static unsigned int crc24q(const unsigned char *buff, int len)
{
    unsigned int crc=0;
    int i;
    
    for (i=0;i<len;i++) crc=((crc<<8)&0xFFFFFF)^tbl_CRC24Q[(crc>>16)^buff[i]];
    return crc;
}

static void setbitu(unsigned char *buff, int pos, int len, unsigned int data)
{
    unsigned int mask=1u<<(len-1);
    int i;
    if (len<=0||32<len) return;
    for (i=pos;i<pos+len;i++,mask>>=1) {
        if (data&mask) buff[i/8]|=1u<<(7-i%8); else buff[i/8]&=~(1u<<(7-i%8));
    }
}
extern void setbits(uint8_t *buff, int pos, int len, int32_t data)
{
    if (data<0) data|=1<<(len-1); else data&=~(1<<(len-1)); /* set sign bit */
    setbitu(buff,pos,len,(uint32_t)data);
}
/* set signed 38 bit field ---------------------------------------------------*/
static void set38bits(uint8_t *buff, int pos, double value)
{
    int word_h=(int)floor(value/64.0);
    uint32_t word_l=(uint32_t)(value-word_h*64.0);
    setbits(buff,pos  ,32,word_h);
    setbitu(buff,pos+32,6,word_l);
}
static unsigned int getbitu(const unsigned char *buff, int pos, int len)
{
    unsigned int bits=0;
    int i;
    for (i=pos;i<pos+len;i++) bits=(bits<<1)+((buff[i/8]>>(7-i%8))&1u);
    return bits;
}
static int getbits(const unsigned char *buff, int pos, int len)
{
    unsigned int bits=getbitu(buff,pos,len);
    if (len<=0||32<=len||!(bits&(1u<<(len-1)))) return (int)bits;
    return (int)(bits|(~0u<<len)); /* extend sign */
}
/* get signed 38bit field ----------------------------------------------------*/
static double getbits_38(const unsigned char *buff, int pos)
{
    return (double)getbits(buff,pos,32)*64.0+getbitu(buff,pos+32,6);
}
/* decode type 1005: stationary rtk reference station arp --------------------*/
extern int decode_type1005_(unsigned char *buff, int len, int *staid, double *pos)
{
    double rr[3];
    int i=24+12,j/*,staid,itrf*/;
    
    if (i+140<=len*8) {
        *staid=getbitu(buff,i,12); i+=12;
        /*itrf =getbitu(buff,i, 6);*/ i+= 6+4;
        rr[0]=getbits_38(buff,i); i+=38+2;
        rr[1]=getbits_38(buff,i); i+=38+2;
        rr[2]=getbits_38(buff,i);
    }
    else {
        return -1;
    }
    
    for (j=0;j<3;j++) {
        pos[j]=rr[j]*0.0001;
    }
    return 5;
}
/* decode type 1006: stationary rtk reference station arp with height --------*/
extern int decode_type1006_(unsigned char *buff, int len, int *staid, double *pos)
{
    double rr[3]/*,anth*/;
    int i=24+12,j/*,staid,itrf*/;
    
    if (i+156<=len*8) {
        *staid=getbitu(buff,i,12); i+=12;
        /*itrf =getbitu(buff,i, 6);*/ i+= 6+4;
        rr[0]=getbits_38(buff,i); i+=38+2;
        rr[1]=getbits_38(buff,i); i+=38+2;
        rr[2]=getbits_38(buff,i); i+=38;
        /*anth =getbitu(buff,i,16);*/
    }
    else {
        return -1;
    }
    
    for (j=0;j<3;j++) {
        pos[j]=rr[j]*0.0001;
    }
    return 5;
}
static int add_rtcm_to_buff(rtcm_buff_t* rtcm, unsigned char data)
{
    if (rtcm->sync == 0) rtcm->slen = 0;
    rtcm->type=0;
    rtcm->crc = 0;
    rtcm->staid = 0;

    if (rtcm->nbyte>=MAX_RTCM_BUF_LEN) rtcm->nbyte = 0;
    if (rtcm->nbyte==0) {
        if (data!=RTCM3PREAMB) return 0;
        rtcm->buff[rtcm->nbyte++]=data;
        return 0;
    }
#if 1
    rtcm->buff[rtcm->nbyte++] = data;
#else
    if (rtcm->mark)
    {
        if (data == 0xEE)
        {
            rtcm->buff[rtcm->nbyte++] = 0x11;
        }
        else if (data == 0xEC)
        {
            rtcm->buff[rtcm->nbyte++] = 0x13;
        }
        else if (data == 0x88)
        {
            rtcm->buff[rtcm->nbyte++] = 0x77;
        }
        else if ((rtcm->nbyte+2) < MAX_RTCM_BUF_LEN)
        {
            rtcm->buff[rtcm->nbyte++] = 0x77;
            rtcm->buff[rtcm->nbyte++] = data;
        }
        rtcm->mark = 0;
    }
    else if (data == 0x77)
    {
        rtcm->mark = 1;
    }
    else
    {
        rtcm->buff[rtcm->nbyte++] = data;
    }
#endif
    return 1;
}

extern int input_rtcm3_type(rtcm_buff_t *rtcm, unsigned char data)
{
    int ret = 0, i = 24, j = 0, mask = 0, is_obs = 0, nbyte = rtcm->nbyte;
    if (rtcm->sync == 0) rtcm->slen = 0;
    if (add_rtcm_to_buff(rtcm, data) == 0) return 0;
    
    if (rtcm->nbyte < 3) return 0;
    rtcm->len=getbitu(rtcm->buff,14,10)+3; /* length without parity */
    if (rtcm->nbyte<rtcm->len+3) return 0;
    nbyte = rtcm->nbyte;
    rtcm->nbyte=0;
    /* check parity */
    if (crc24q(rtcm->buff, rtcm->len) != getbitu(rtcm->buff, rtcm->len * 8, 24)) {
        rtcm->crc = 1;
        return 0;
    }
    i = 24;
    rtcm->type = getbitu(rtcm->buff, i, 12); i += 12;
    /* decode rtcm3 message */
    if ((rtcm->type == 1074 || rtcm->type == 1075 || rtcm->type == 1076 || rtcm->type == 1077)|| /* GPS */
        (rtcm->type == 1094 || rtcm->type == 1095 || rtcm->type == 1096 || rtcm->type == 1097)|| /* GAL */
        (rtcm->type == 1104 || rtcm->type == 1105 || rtcm->type == 1106 || rtcm->type == 1107)|| /* SBS */
        (rtcm->type == 1114 || rtcm->type == 1115 || rtcm->type == 1116 || rtcm->type == 1117))   /* QZS */
    {
        /* GPS, GAL, SBS, QZS */
        rtcm->staid = getbitu(rtcm->buff, i, 12);           i += 12;
        rtcm->tow   = getbitu(rtcm->buff, i, 30) * 0.001;   i += 30;
        rtcm->sync  = getbitu(rtcm->buff, i,  1);           i +=  1;
        ret = rtcm->sync?0:1;
        is_obs = 1;
    }
    if (rtcm->type == 1084 || rtcm->type == 1085 || rtcm->type == 1086 || rtcm->type == 1087)
    {
        /* GLO */
        rtcm->staid = getbitu(rtcm->buff, i, 12);           i += 12;
        double dow  = getbitu(rtcm->buff, i,  3);           i +=  3;
        double tod  = getbitu(rtcm->buff, i, 27) * 0.001;   i += 27;
        rtcm->sync  = getbitu(rtcm->buff, i,  1);           i +=  1;
        double tow  = dow * 24 * 3600 + tod - 3 * 3600 + 18;
        if (rtcm->tow>0.0&&fabs(tow- rtcm->tow)>(24*1800))
        {
            tow -= floor((tow- rtcm->tow)/(24*1800))*(24*1800);
        }
        tow -= floor(tow / 604800) * 604800;
        rtcm->tow   = tow;
        ret = rtcm->sync?0:1;
        is_obs = 1;
    }
    if (rtcm->type == 1124 || rtcm->type == 1125 || rtcm->type == 1126 || rtcm->type == 1127)
    {
        /* BDS */
        rtcm->staid = getbitu(rtcm->buff, i, 12);           i += 12;
        double tow  = getbitu(rtcm->buff, i, 30) * 0.001;   i += 30;
        rtcm->sync  = getbitu(rtcm->buff,i, 1);             i +=  1;
        tow += 14.0; /* BDT -> GPST */
        tow -= floor(tow / 604800) * 604800;
        rtcm->tow = tow;
        ret = rtcm->sync?0:1;
        is_obs = 1;
    }
    if (is_obs)
    {
        memset(rtcm->sats, 0, sizeof(rtcm->sats));
        memset(rtcm->sigs, 0, sizeof(rtcm->sigs));
        memset(rtcm->cels, 0, sizeof(rtcm->cels));
        rtcm->nsat = 0;
        rtcm->nsig = 0;
        rtcm->ncel = 0;
        /* iod */      i += 3;
        /* time_s*/    i += 7;
        /* clk_str */  i += 2;
        /* clk_ext */  i += 2;
        /* smooth */   i += 1;
        /* tint_s */   i += 3;
        for (j = 1; j <= 64; j++) {
            mask = getbitu(rtcm->buff, i, 1); i += 1;
            if (mask) rtcm->sats[rtcm->nsat++] = j;
        }
        for (j = 1; j <= 32; j++) {
            mask = getbitu(rtcm->buff, i, 1); i += 1;
            if (mask) rtcm->sigs[rtcm->nsig++] = j;
        }
        if (i + rtcm->nsat * rtcm->nsig > rtcm->len * 8) {
            /* error */
            rtcm->nsat = rtcm->nsig = rtcm->ncel = 0;
        }
        else
        {
            for (j = 0; j < rtcm->nsat * rtcm->nsig; j++) {
                rtcm->cels[j] = getbitu(rtcm->buff, i, 1); i += 1;
                if (rtcm->cels[j]) rtcm->ncel++;
            }
        }
#if 0 //_WIN32
        printf("%10.3f,%4i,%4i,%4i,%4i,%4i,%4i,SAT:", rtcm->tow, rtcm->type, nbyte, rtcm->len + 3, rtcm->nsat, rtcm->nsig, rtcm->ncel);
        for (j = 0; j < 64; ++j)
        {
            if (rtcm->sats[j])
            {
                printf(",%2i", j + 1);
            }
        }
        printf(",SIG:");
        for (j = 0; j < 32; ++j)
        {
            if (rtcm->sigs[j])
            {
                printf(",%2i", j + 1);
            }
        }
        printf("\n");
#endif
        /* MSM4 => nsat * (8  +10   ) + ncel * (15+22+ 4+1+ 6   ) = nsat *18 + ncel *48 */
        /* MSM5 => nsat * (8+4+10+14) + ncel * (15+22+ 4+1+ 6+15) = nsat *36 + ncel *63 */
        /* MSM6 => nsat * (8  +10   ) + ncel * (20+24+10+1+10   ) = nsat *18 + ncel *65 */
        /* MSM7 => nsat * (8+4+10+14) + ncel * (20+24+10+1+10+15) = nsat *36 + ncel *80 */
    }
    if (rtcm->type == 1019)
    {
        int prn   =getbitu(rtcm->buff,i, 6);              i+= 6;
        int week  =getbitu(rtcm->buff,i,10);              i+=10;
        rtcm->wk  =week+2048;
        rtcm->sys = 'G';
        rtcm->prn = prn;
    }
    if (rtcm->type == 1020)
    {
        int prn   =getbitu(rtcm->buff,i, 6);              i+= 6;
        int frq   =getbitu(rtcm->buff,i, 5)-7;            i+= 5+2+2;
        rtcm->sys = 'R';
        rtcm->prn = prn;
    }
    if (rtcm->type == 1042)
    {
        int prn   =getbitu(rtcm->buff,i, 6);              i+= 6;
        int week  =getbitu(rtcm->buff,i,13);              i+=13;
        rtcm->wk  =week+1356; /* BDT week to GPS week */
    }
    if (rtcm->type == 1044)
    {
        int prn   =getbitu(rtcm->buff,i, 4);              i+= 4+430;
        int week  =getbitu(rtcm->buff,i,10);              i+=10;
        rtcm->wk  =week+2048;
        rtcm->sys = 'J';
        rtcm->prn = prn;
    }		
    if (rtcm->type == 1045|| rtcm->type == 1046)
    {
        int prn   =getbitu(rtcm->buff,i, 6);              i+= 6;
        int week  =getbitu(rtcm->buff,i,12);              i+=12; /* gst-week */
        rtcm->wk  =week+1024 ; /* gal-week = gst-week + 1024 */
    }
    if (rtcm->type == 1005)
    {
        ret = decode_type1005_(rtcm->buff, rtcm->len, &rtcm->staid, rtcm->pos);
    }
    if (rtcm->type == 1006)
    {
        ret = decode_type1006_(rtcm->buff, rtcm->len, &rtcm->staid, rtcm->pos);
    }
    if (rtcm->type == 1033)
    {
        ret = decode_type1033_(rtcm->buff, rtcm->len, rtcm->staname, rtcm->antdes, rtcm->antsno, rtcm->rectype, rtcm->recver, rtcm->recsno);
    }
    rtcm->slen += rtcm->len + 3;
    return ret;
}

extern int rtcm_obs_type(int type)
{
    if ((type == 1074 || type == 1075 || type == 1076 || type == 1077) || /* GPS */
        (type == 1084 || type == 1085 || type == 1086 || type == 1087) || /* GLO */
        (type == 1094 || type == 1095 || type == 1096 || type == 1097) || /* GAL */
        (type == 1104 || type == 1105 || type == 1106 || type == 1107) || /* SBS */
        (type == 1114 || type == 1115 || type == 1116 || type == 1117) || /* QZS */
        (type == 1124 || type == 1125 || type == 1126 || type == 1127))  /* BDS */
        return 1;
    else
        return 0;
}
extern int rtcm_eph_type(int type)
{
    return type == 1019 || type == 1020 || type == 1042 || type == 1044 || type == 1045 || type == 1046;
}

extern int update_type_1005_1006_pos(uint8_t* buff, int nbyte, double* p)
{
    int len = 0, i = 24, type = 0, staid = 0;
    int crc = 0;
    int ret = 0;
    if (buff[0] != RTCM3PREAMB || nbyte < 6) return ret;
    len = getbitu(buff, 14, 10) + 3; /* length without parity */
    if (nbyte < (len + 3)) return ret;

    i = 24; /* type */
    type = getbitu(buff, i, 12); i += 12;

    if (type == 1005 || type == 1006)
    {
        /* 24 + 12, staid */
        i += 12; /* ref station id */
        i += 6; /* itrf realization year */
        i += 1; /* gps indicator */
        i += 1; /* glonass indicator */
        i += 1; /* galileo indicator */
        i += 1; /* ref station indicator */
        set38bits(buff, i, p[0] / 0.0001); i += 38; /* antenna ref point ecef-x */
        i += 1; /* oscillator indicator */
        i += 1; /* reserved */
        set38bits(buff, i, p[1] / 0.0001); i += 38; /* antenna ref point ecef-y */
        i += 2; /* quarter cycle indicator */
        set38bits(buff, i, p[2] / 0.0001); i += 38; /* antenna ref point ecef-z */
        if (type==1006)
        {
        setbitu  (buff, i, 16,         0); i += 16; /* antenna height */
        }
        /* crc-24q */
        crc = crc24q(buff, len);
        setbitu(buff, len * 8, 24, crc);
        ret = 1;
    }
    return ret;
}
/* decode type 1033: receiver and antenna descriptor -------------------------*/
extern int decode_type1033_(uint8_t* buff, int len, char *staname, char* antdes, char* antsno, char* rectype, char* recver, char* recsno)
{
    char des[32]="",sno[32]="",rec[32]="",ver[32]="",rsn[32]="";
    int i=24+12,j,staid,n,m,n1,n2,n3,setup;
    
    n =getbitu(buff,i+12,8);
    m =getbitu(buff,i+28+8*n,8);
    n1=getbitu(buff,i+36+8*(n+m),8);
    n2=getbitu(buff,i+44+8*(n+m+n1),8);
    n3=getbitu(buff,i+52+8*(n+m+n1+n2),8);
    
    if (i+60+8*(n+m+n1+n2+n3)<=len*8) {
        staid=getbitu(buff,i,12); i+=12+8;
        for (j=0;j<n&&j<31;j++) { 
            des[j]=(char)getbitu(buff,i,8); i+=8;
        }
        setup=getbitu(buff,i, 8); i+=8+8;
        for (j=0;j<m&&j<31;j++) {
            sno[j]=(char)getbitu(buff,i,8); i+=8;
        }
        i+=8;
        for (j=0;j<n1&&j<31;j++) {
            rec[j]=(char)getbitu(buff,i,8); i+=8;
        }
        i+=8;
        for (j=0;j<n2&&j<31;j++) {
            ver[j]=(char)getbitu(buff,i,8); i+=8;
        }
        i+=8;
        for (j=0;j<n3&&j<31;j++) {
            rsn[j]=(char)getbitu(buff,i,8); i+=8;
        }
    }
    else {
        return -1;
    }
    
    sprintf(staname,"%04d",staid);
    strncpy(antdes, des,n ); antdes [n] ='\0';
    strncpy(antsno, sno,m ); antsno [m] ='\0';
    strncpy(rectype,rec,n1); rectype[n1]='\0';
    strncpy(recver, ver,n2); recver [n2]='\0';
    strncpy(recsno, rsn,n3); recsno [n3]='\0';
    
    //printf("rtcm3 1033: ant=%s:%s rec=%s:%s:%s\n",des,sno,rec,ver,rsn);
    return 5;
}