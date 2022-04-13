// test_rtcm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>

#include "hy_rtcm_buff.h"

typedef struct
{
    int type;
    double time;
    unsigned long numofepoch;
}rtcm_obs_t;

static void test_rtcm(const char* fname)
{
    FILE* fRTCM = fopen(fname, "rb");

    if (fRTCM==NULL) return;

    int data = 0;
    rtcm_buff_t gRTCM_Buf = { 0 };
    rtcm_buff_t* rtcm = &gRTCM_Buf;
    unsigned long numofcrc = 0;
    unsigned long numofepoch = 0;
    double lastTime = 0.0;
    std::vector< rtcm_obs_t> vObsType;
    while (!feof(fRTCM))
    {
        if ((data = fgetc(fRTCM)) == EOF) break;
        int ret = input_rtcm3_type(rtcm, (unsigned char)data);
        if (rtcm->type > 0) /* rtcm data */
        {
            if (rtcm->crc == 1)
            {
                ++numofcrc;
                continue;
            }
            if (ret == 1)
            {
                if (numofepoch > 0)
                {
                    double dt = rtcm->tow - lastTime;
                    if (dt > 1.5 && rtcm_obs_type(rtcm->type))
                    {
                        printf("%10.3f,%10.3f,%6i,%4i, current time, data gap, number of epoch, data type\r\n", rtcm->tow, dt, numofepoch, rtcm->type);
                    }
                }
                ++numofepoch;
                lastTime = rtcm->tow;
            }
            int i = 0;
            for (; i < vObsType.size(); ++i)
            {
                if (vObsType[i].type == rtcm->type)
                {
                    if (vObsType[i].numofepoch > 0)
                    {
                        double dt = rtcm->tow - vObsType[i].time;
                        if (dt > 1.5 && rtcm_obs_type(rtcm->type))
                        {
                            printf("%10.3f,%10.3f,%6i,%4i, current time, data gap, number of epoch, data type\r\n", rtcm->tow, dt, vObsType[i].numofepoch, rtcm->type);
                        }
                    }
                    vObsType[i].numofepoch++;
                    vObsType[i].time = rtcm->tow;
                    break;
                }
            }
            if (i == vObsType.size())
            {
                rtcm_obs_t temp = { 0 };
                temp.type = rtcm->type;
                temp.time = rtcm->tow;
                temp.numofepoch = 1;
                vObsType.push_back(temp);
            }
        }
    }
    for (int i = 0; i < vObsType.size(); ++i)
    {
        printf("%4i,%6u,%6u,%s, rtcm data type, total epoch for current type, total epoch based on sync flag\r\n", vObsType[i].type, vObsType[i].numofepoch, numofepoch, fname);
    }
    printf("%6u,%s, failed in CRC check\r\n", numofcrc, fname);
    if (fRTCM) fclose(fRTCM);
}

int main(int argc, const char* argv[])
{
    if (argc>1)
        test_rtcm(argv[1]);
    //test_rtcm("D:\\data\\rtklib_bin\\2022-1-29-20-19-37-COM8.bin");
}

