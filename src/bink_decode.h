#ifndef BINK_DECODE_H
#define BINK_DECODE_H

#include <errno.h>
#include <libavcodec/avcodec.h>

typedef int (*BinkFrameConsumer)(void *opaque, AVFrame *frame);

static int BinkReceiveFrames(AVCodecContext *codec, AVFrame *scratch,
                            BinkFrameConsumer consume, void *opaque)
{
    int count = 0;
    int result;
    while ((result = avcodec_receive_frame(codec, scratch)) == 0)
    {
        result = consume(opaque, scratch);
        if (result < 0)
            return result;
        count++;
    }
    return result == AVERROR(EAGAIN) || result == AVERROR_EOF ? count : result;
}

static int BinkDecodePacket(AVCodecContext *codec, const AVPacket *packet,
                           AVFrame *scratch, BinkFrameConsumer consume, void *opaque)
{
    int count = 0;
    int result = avcodec_send_packet(codec, packet);
    if (result == AVERROR(EAGAIN))
    {
        result = BinkReceiveFrames(codec, scratch, consume, opaque);
        if (result <= 0)
            return result < 0 ? result : AVERROR_BUG;
        count = result;
        result = avcodec_send_packet(codec, packet);
    }
    if (result == AVERROR_EOF)
        return count;
    if (result < 0)
        return result;
    result = BinkReceiveFrames(codec, scratch, consume, opaque);
    return result < 0 ? result : count + result;
}

#endif
