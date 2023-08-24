#pragma pack(4)
#define _SYS__TIMEVAL_H_
struct timeval {
    unsigned int tv_sec;
    unsigned int tv_usec;
};

#include "user_lib.h"
#include "videodev2.h"
#define DARK 64
#define RATIO 7 / 10

int main() {
    char *info = allocate_share_page();
    int pid = fork();
    if (pid == 0) {
        int f = open_u("/dev/video0", O_RDWR), r;

        struct v4l2_format fmt;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.width = 320;
        fmt.fmt.pix.height = 180;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
        r = ioctl_u(f, VIDIOC_S_FMT, &fmt);
        printu("Pass format: %d\n", r);

        struct v4l2_requestbuffers req;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.count = 1; req.memory = V4L2_MEMORY_MMAP;
        r = ioctl_u(f, VIDIOC_REQBUFS, &req);
        printu("Pass request: %d\n", r);

        struct v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP; buf.index = 0;
        r = ioctl_u(f, VIDIOC_QUERYBUF, &buf);
        printu("Pass buffer: %d\n", r);

        int length = buf.length;
        char *img = mmap_u(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, f, buf.m.offset);
        unsigned int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        r = ioctl_u(f, VIDIOC_STREAMON, &type);
        printu("Open stream: %d\n", r);

        char *img_data = naive_malloc();
        for (int i = 0; i < (length + 4095) / 4096 - 1; i++)
            naive_malloc();
        yield();

        for (;;) {
            if (*info == '1') {
                r = ioctl_u(f, VIDIOC_QBUF, &buf);
                printu("Buffer enqueue: %d\n", r);
                r = ioctl_u(f, VIDIOC_DQBUF, &buf);
                printu("Buffer dequeue: %d\n", r);
                r = read_mmap_u(img_data, img, length);
                int num = 0;
                for (int i = 0; i < length; i += 2)
                    if (img_data[i] < DARK) num++;
                printu("Dark num: %d > %d\n", num, length / 2 * RATIO);
                if (num > length / 2 * RATIO) {
                    *info = '0'; car_control('0');
                }
            } else if (*info == 'q') break;
        }

        for (char *i = img_data; i - img_data < length; i += 4096)
            naive_free(i);
        r = ioctl_u(f, VIDIOC_STREAMOFF, &type);
        printu("Close stream: %d\n", r);
        munmap_u(img, length);
        close(f);
        exit(0);
    } else {
        yield();
        while(1)
        {
            char temp = (char)uartgetchar();
            *info = temp;
            if(temp == 'q')
                break;
            car_control(temp);
        }
    }
    return 0;
}
