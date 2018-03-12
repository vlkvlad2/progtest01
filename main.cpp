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
                int16_t & interleaveBegin,
                int16_t & size,
                int16_t & final,
                int & interleave)
{
    ifstream from( srcFileName, ios::in | ios::binary);
    int16_t temp = 0;
    int16_t tmpChannel = 0;
    int16_t interleaveBegintmp = 0;
    int16_t head[8];
    int16_t tmpFinal, first, second, third;
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

    tmpFinal = head[6];
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

    from.seekg(0,ios_base::end);
    size = from.tellg();

    size = size - 8;
    if (size != (height * channel * width))
        return false;


    final = tmpFinal | 224;

    first = interleave << 5;

    second = interleave & 2;
    second = second >> 1;

    cout << first << " " << second << endl;


    cout << final << endl;
    from.close();

    return true;

}


bool readPicture (const char * srcFileName,
                  int16_t & height,
                  int16_t & width,
                  int16_t ** picture,
                  bool ** isFilled,
                  int16_t & channel,
                  int16_t & interleaveBegin)
{
    ifstream from(srcFileName, ios::in | ios::binary);
    int16_t tmp = 0;
    from.seekg(8, ios::beg);

    if (!from.good())
        return false;

   for (int16_t k = interleaveBegin; k > 0; k = k / 2)
    {
        for (int16_t i = 0; i < height; i = i + k)
        {
            for (int16_t j = 0; j < width * channel; j = j + k) {

                 if (isFilled[i][j] != true) {
                     if (!from.read((char *) &tmp, 1))
                         return false;
                     picture[i][j] = tmp;
                     isFilled[i][j] = true;
                 }
            }
        }

    }

    for (int16_t i = 0; i < height; i++)
    {
        for (int16_t j = 0; j < width * channel; j++)
        {
            cout << picture [i][j] << " ";

        }
        cout << endl;
    }
    from.close();
    return true;

}

bool interleavePicture (const char * dstFileName,
                        int16_t & height,
                        int16_t & width,
                        int16_t & channel,
                        int16_t ** picture,
                        int & interleave,
                        bool ** isFilled)
{
    int zero = 0;
    ofstream of;
    of.open (dstFileName, ios::binary);
    if (! of.good())
        return false;

    of.write((char*)&ENDIAN_LITTLE, 2);
    of.write((char*)&height, 2);
    of.write((char*)&width, 2);
    of.write((char*)&zero, 1);
    of.write((char*)&zero, 1);


    for (int16_t k = interleave; k > 0; k = k / 2)
    {
        for (int16_t i = 0; i < height; i = i + k)
        {
            for (int16_t j = 0; j < width * channel; j = j + k) {
                if (isFilled[i][j] != true) {
                    if (! of.write((char*)&picture[i][j], 1))
                        return false;
                    isFilled[i][j] = true;
                }
            }
        }
    }

    of.close();
    return true;
}

bool recodeImage (const char  * srcFileName,
                  const char  * dstFileName,
                  int           interleave,
                  uint16_t      byteOrder)
{
    int16_t height = 0, width = 0, channel = 0, bytesPerChannel = 0, interleaveBegin = 0, size = 0;
    int16_t ** picture;
    bool ** isFilled;
    int16_t final = 0;

    readHead (srcFileName, height, width, channel, bytesPerChannel, interleaveBegin, size, final, interleave);

    picture = new int16_t*[height];
    for (int i = 0; i<height; i++)
    {
        picture[i] = new int16_t [width * channel];
        for (int j = 0; j < width * channel; j++)
        {
            picture [i][j] = 0;
        }
    }

    isFilled = new bool*[height];
    for (int i = 0; i<height; i++)
    {
        isFilled[i] = new bool[width * channel];
        for (int j = 0; j < width * channel; j++)
        {
            isFilled [i][j] = false;
        }
    }

    readPicture (srcFileName, height, width, picture, isFilled, channel, interleaveBegin);

    for (int i = 0; i<height; i++)
    {
        for (int j = 0; j < width * channel; j++) {
            isFilled[i][j] = false;
        }
    }


    interleavePicture (dstFileName, height, width, channel, picture, interleave, isFilled);

    for (int i = 0; i < height; i++)
    {
        delete [] picture[i];

    }
    delete [] picture;



    return true;

}

int main() {

    recodeImage("pr01/input_03.img", "aaa", 2, 0);

    return 0;
}