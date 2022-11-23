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

typedef struct
{
    double x;
    double y;
    double z;
}xyz_t;

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
    std::vector< xyz_t> vxyz;
    while (!feof(fRTCM))
    {
        if ((data = fgetc(fRTCM)) == EOF) break;
        int ret = input_rtcm3_type(rtcm, (unsigned char)data);
        if (rtcm->type > 0) /* rtcm data */
        {
            printf("%10.3f,%4i,%i,%i\n", rtcm->tow, rtcm->type, rtcm->sync, rtcm->crc);
            if (rtcm->crc == 1)
            {
                ++numofcrc;
                continue;
            }
            if (ret == 5)
            {
                printf("%14.4f,%14.4f,%14.4f\n", rtcm->pos[0], rtcm->pos[1], rtcm->pos[2]);
                if ((rtcm->pos[0] * rtcm->pos[0] + rtcm->pos[1] * rtcm->pos[1] + rtcm->pos[2] * rtcm->pos[2]) > 0.1)
                {
                    xyz_t xyz = { 0 };
                    xyz.x = rtcm->pos[0];
                    xyz.y = rtcm->pos[1];
                    xyz.z = rtcm->pos[2];
                    if (vxyz.size() > 0)
                    {
                        std::vector<xyz_t>::reverse_iterator pxyz = vxyz.rbegin();
                        double dxyz[3] = { pxyz->x - xyz.x, pxyz->y - xyz.y, pxyz->z - xyz.z };
                        if (sqrt(dxyz[0] * dxyz[0] + dxyz[1] * dxyz[1] + dxyz[2] * dxyz[2]) > 0.001)
                        {
                            vxyz.push_back(xyz);
                        }
                    }
                    else
                    {
                        vxyz.push_back(xyz);
                    }
                }
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
    if (vxyz.size() > 0)
    {
        printf("%.4f,%.4f,%.4f\n", vxyz.rbegin()->x, vxyz.rbegin()->y, vxyz.rbegin()->z);
    }
    if (fRTCM) fclose(fRTCM);
}

int main(int argc, const char* argv[])
{
    if (argc>1)
        test_rtcm(argv[1]);
    return 0;
}

