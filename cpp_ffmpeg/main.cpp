#include <iostream>
// 包含ffmpeg头文件


//#include "libavutil/avutil.h"

#ifdef __cplusplus  ///
extern "C"
{
// 包含ffmpeg头文件
#include "libavutil/avutil.h"
}
#endif


using namespace std;



int main()
{
    printf("Hello FFMPEG, version is %s\n", av_version_info());

    return 0;
}
