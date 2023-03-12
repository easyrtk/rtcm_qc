// test_rtcm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>

#include "rtcm_buff.h"

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

#ifndef ae_WGS84
#define ae_WGS84 6378137.0
#endif

#ifndef finv_WGS84
#define finv_WGS84 298.257223563
#endif

#ifndef PI
#define	PI 3.14159265358979
#endif

double lat2local(double lat, double* lat2north)
{
    double f_WGS84 = (1.0 / finv_WGS84);
    double e2WGS84 = (2.0 * f_WGS84 - f_WGS84 * f_WGS84);
    double slat = sin(lat);
    double clat = cos(lat);
    double one_e2_slat2 = 1.0 - e2WGS84 * slat * slat;
    double Rn = ae_WGS84 / sqrt(one_e2_slat2);
    double Rm = Rn * (1.0 - e2WGS84) / (one_e2_slat2);
    *lat2north = Rm;
    return Rn * clat;
}


extern void xyz2blh_(const double* xyz, double* blh)
{
    // ecef xyz => blh
    double a = ae_WGS84, finv = finv_WGS84;
    double f = 1.0 / finv, e2 = 2 * f - f * f;
    double x = xyz[0], y = xyz[1], z = xyz[2], lat, lon, ht;
    double R = sqrt(x * x + y * y + z * z);
    double ang = 0.0;
    double lat1 = 0.0;
    double Rw = 0.0;
    double Rn = 0.0;
    if (fabs(z) < 1.0e-5)
    {
        lat = 0.0;
    }
    else
    {
        ang = atan(fabs(z / sqrt(x * x + y * y))) * ((z < 0.0) ? -1.0 : 1.0);
        //if (z<0.0) ang = -ang;
        lat1 = ang;
        Rw = sqrt(1 - e2 * sin(lat1) * sin(lat1));
        Rn = 0.0;
        lat = atan(fabs(tan(ang) * (1 + (a * e2 * sin(lat1)) / (z * Rw))));
        if (z < 0.0) lat = -lat;
        while (fabs(lat - lat1) > 1e-12)
        {
            lat1 = lat;
            Rw = sqrt(1 - e2 * sin(lat1) * sin(lat1));
            lat = atan(fabs(tan(ang) * (1 + (a * e2 * sin(lat1)) / (z * Rw))));
            if (z < 0.0) lat = -lat;
        }
        if (lat > PI) lat = lat - 2.0 * PI;
    }
    if (fabs(x) < 1e-5)
    {
        if (y >= 0.0)
            lon = PI / 2.0;
        else
            lon = 3.0 * PI / 2.0;
    }
    else
    {
        lon = atan(fabs(y / x));
        if (x > 0.0)
        {
            if (y >= 0.0)
                lon = lon;
            else
                lon = 2.0 * PI - lon;
        }
        else
        {
            if (y >= 0.0)
                lon = PI - lon;
            else
                lon = PI + lon;
        }
    }
    Rw = sqrt(1 - e2 * sin(lat) * sin(lat));
    Rn = a / Rw;
    ht = R * cos(ang) / cos(lat) - Rn;
    if (lon > PI) lon = lon - 2.0 * PI;
    blh[0] = lat;
    blh[1] = lon;
    blh[2] = ht;
    return;
}

void blh2xyz_(const double* blh, double* xyz)
{
    // lat, lon, ht => ecef xyz
    double a = ae_WGS84, finv = finv_WGS84;
    double f = 1.0 / finv, e2 = 2 * f - f * f;
    double lat = blh[0], lon = blh[1], ht = blh[2];
    double Rw = sqrt(1 - e2 * sin(lat) * sin(lat));
    double Rn = a / Rw;
    xyz[0] = (Rn + ht) * cos(lat) * cos(lon);
    xyz[1] = (Rn + ht) * cos(lat) * sin(lon);
    xyz[2] = (Rn * (1 - e2) + ht) * sin(lat);
    return;
}
static void test_rtcm(const char* fname)
{
    FILE* fRTCM = fopen(fname, "rb");

    if (fRTCM==NULL) return;

    int data = 0;
    rtcm_buff_t gRTCM_Buf = { 0 };
    rtcm_buff_t* rtcm = &gRTCM_Buf;
    unsigned long numofcrc = 0;
    unsigned long numofepoch = 0;
    unsigned long numofepoch_bad = 0;
    double lastTime = 0.0;
    std::vector< rtcm_obs_t> vObsType;
    std::vector<xyz_t> vxyz;

    std::vector<std::string> vDataGapOutput;
    std::vector<std::string> vTypeGapOutput;
    double start_time =-1;
    double end_time = -1;
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
            if (rtcm_obs_type(rtcm->type))
            {
                if (start_time < 0) start_time = rtcm->tow;
                end_time = rtcm->tow;
                int hour = rtcm->tow / 3600;
                double sec = rtcm->tow - 3600 * hour;
                hour %= 24;
                int mm = sec / 60;
                sec -= mm * 60;
                //printf("%10.3f,%2i,%2i,%2i,%4i,%4i,%4i\n", rtcm->tow, hour, mm, (int)sec, rtcm->type, rtcm->nbyte, numofepoch);
            }
            double dt = rtcm->tow - lastTime;
            if (ret == 1)
            {
                if (numofepoch > 0)
                {
                    if (dt > 1.5 && rtcm_obs_type(rtcm->type))
                    {
                        char temp[255] = { 0 };
                        sprintf(temp, "%10.3f,%10.3f,%6i\r\n", rtcm->tow, dt, numofepoch);
                        vDataGapOutput.push_back(temp);
                    }
                    if (dt < 0.8)
                    {
                        dt = dt;
                        ++numofepoch_bad;
                    }
                    else
                    {
                        ++numofepoch;
                    }
                }
                else
                {
                    ++numofepoch;
                }
                lastTime = rtcm->tow;
            }
            if (ret == 5)
            {
                xyz_t xyz = { 0 };
                xyz.x = rtcm->pos[0];
                xyz.y = rtcm->pos[1];
                xyz.z = rtcm->pos[2];
                vxyz.push_back(xyz);
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
                            char temp[255] = { 0 };
                            sprintf(temp, "%10.3f,%10.3f,%6i,%4i\r\n", rtcm->tow, dt, vObsType[i].numofepoch, rtcm->type);
                            vTypeGapOutput.push_back(temp);
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
    if (end_time < start_time) end_time += 7 * 24 * 3600;
    int total_epoch = (int)(end_time - start_time + 1);
    printf("rtcm stats for %s,%10.3f,%10.3f,%i,%i,%i,%7.3f,%i\r\n", fname, start_time, end_time, total_epoch, numofepoch, total_epoch - numofepoch, 100 - (total_epoch > 0) ? (numofepoch * 100.0 / total_epoch) : (0), numofepoch_bad);
    printf("%s\n%s\n%s\n%s\n%s\n%s\n", rtcm->staname, rtcm->antdes, rtcm->antsno, rtcm->rectype, rtcm->recver, rtcm->recsno);
    if (vObsType.size() > 0)
    {
        printf("RTCM TYPE Count\n");
        for (int i = 0; i < vObsType.size(); ++i)
        {
            printf("%4i,%6u,%6u\r\n", vObsType[i].type, vObsType[i].numofepoch, numofepoch);
        }
    }
    printf("%6u, failed in CRC check\r\n", numofcrc);
    double midXYZ[3] = { 0 };
    for (int i = 0; i < vxyz.size(); ++i)
    {
        midXYZ[0] += vxyz[i].x;
        midXYZ[1] += vxyz[i].y;
        midXYZ[2] += vxyz[i].z;
    }
    if (vxyz.size() > 0)
    {
        midXYZ[0] /= vxyz.size();
        midXYZ[1] /= vxyz.size();
        midXYZ[2] /= vxyz.size();
        double stdXYZ[3] = { 0 };
        double midBLH[3] = { 0 };
        xyz2blh_(midXYZ, midBLH);
        if (vxyz.size() > 1)
        {
            for (int i = 0; i < vxyz.size(); ++i)
            {
                stdXYZ[0] += (vxyz[i].x - midXYZ[0]) * (vxyz[i].x - midXYZ[0]);
                stdXYZ[1] += (vxyz[i].y - midXYZ[1]) * (vxyz[i].y - midXYZ[1]);
                stdXYZ[2] += (vxyz[i].z - midXYZ[2]) * (vxyz[i].z - midXYZ[2]);
            }
            stdXYZ[0] = sqrt(stdXYZ[0] / (vxyz.size() - 1));
            stdXYZ[1] = sqrt(stdXYZ[1] / (vxyz.size() - 1));
            stdXYZ[2] = sqrt(stdXYZ[2] / (vxyz.size() - 1));
            printf("%.9f,%.9f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%i\n", midBLH[0] * 180 / PI, midBLH[1] * 180 / PI, midBLH[2], midXYZ[0], midXYZ[1], midXYZ[2], stdXYZ[0], stdXYZ[1], stdXYZ[2], (int)vxyz.size());
        }
        else
        {
            printf("%.9f,%.9f,%.4f,%.4f,%.4f,%.4f,%i\n", midBLH[0]*180/PI, midBLH[1] * 180 / PI, midBLH[2], midXYZ[0], midXYZ[1], midXYZ[2], (int)vxyz.size());
        }
        int ii = 0;
    }
    if (vDataGapOutput.size() > 0)
    {
        printf("Epoch Data GAP\n");
        for (int i = 0; i < vDataGapOutput.size(); ++i)
        {
            printf("%s", vDataGapOutput[i].c_str());
        }
    }
    if (vTypeGapOutput.size() > 0)
    {
        printf("RTCM TYPE GAP\n");
        for (int i = 0; i < vTypeGapOutput.size(); ++i)
        {
            printf("%s", vTypeGapOutput[i].c_str());
        }
    }
    if (fRTCM) fclose(fRTCM);
}

int main(int argc, const char* argv[])
{
    if (argc>1)
        test_rtcm(argv[1]);
    return 0;
}

