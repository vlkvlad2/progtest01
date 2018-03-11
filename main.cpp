#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cctype>
#include <climits>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <algorithm>
#include <functional>
#include <memory>
using namespace std;

const uint16_t ENDIAN_LITTLE = 0x4949;
const uint16_t ENDIAN_BIG    = 0x4d4d;

bool readHead ( const char * srcFileName,
                int16_t & height,
                int16_t & width,
                int16_t & channel,
                int16_t & bytesPerChannel,
                int16_t & interleaveBegin )
{
    ifstream from( srcFileName, ios::in | ios::binary);
    int16_t temp = 0;
    int16_t tmpChannel = 0;
    int16_t interleaveBegintmp = 0;
    int16_t head[8];
    int tmp[] = { 0, 2, 3, 4, 5};

    if(!from.good())
        return false;
    for(int i=0; i < 8; i++)
    {
        if (!(from.read((char*)&temp,1)))
            return false;
        head[i]=temp;
    }

    if (head[0] != 73)
        return false;
    if (head[1] != 73)
        return false;

    width = head[3]<<8;
    height = head[5]<<8;
    width = width + head[2];
    height = height + head[4];

    tmpChannel = head[6] & 3;

    switch (tmpChannel) {
        case 0:
            channel = 1;
            break;
        case 1:
            return false;

        case 2:
            channel = 3;
            break;
        case 3:
            channel = 4;
            break;
        default:
            return false;

    }

    bytesPerChannel = head[6] >> 2;
    bytesPerChannel = bytesPerChannel & 7;

    for(int i=0; i<5; i++)
    {
        if (tmp[i]==bytesPerChannel)
        {
            bytesPerChannel=pow(2,tmp[i]);
            break;
        }else if(tmp[i] == 5)
        {
            return false;
        }
    }


    if (bytesPerChannel != 8)
        return false;

    interleaveBegintmp = head[6] >> 5;
    interleaveBegintmp = interleaveBegintmp & 7;
    interleaveBegin = pow(2,interleaveBegintmp);

    if(interleaveBegin == 128)
        return false;
    if(head[6] >> 7 != 0)
        return false;
    if(head[7] != 0)
        return false;

    cout << width << " " << height << " " << bytesPerChannel << " " << channel << " " << interleaveBegin  << endl;

    return true;

}


bool recodeImage (const char  * srcFileName,
                  const char  * dstFileName,
                  int           interleave,
                  uint16_t      byteOrder)
{
    int16_t height, width, channel, bytesPerChannel, interleaveBegin;
    readHead (srcFileName, height, width, channel, bytesPerChannel, interleaveBegin);
    return true;

}

int main() {

    recodeImage("pr01/ref_01.img", "aaa", 0, 0);

    return 0;
}