#include <assert.h>
#include <stdbool.h>

#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>

#include "stream.h"

#define STREAM_DEV "/dev/video0"
#define STREAM_REMOTE "tcp://192.168.0.3"

static AVFormatContext *out, *camera;

void stream_init(void)
{
    avformat_network_init();
    camera = avformat_alloc_context();
    AVDictionary *input_opts = NULL;
    av_dict_set(&input_opts, "video_size", "1920x1080", 0);
    const AVInputFormat *h264 = av_find_input_format("h264");
    if(!h264)
        fprintf(stderr, "FFmpeg doesn't support h264\n");
    int ret = avformat_open_input(&camera, STREAM_DEV, h264, &input_opts);
    if(ret < 0)
        fprintf(stderr, "avformat_open_input() failed\n");
    avformat_find_stream_info(camera, NULL);
    assert(camera->nb_streams == 1);
    AVStream *video = camera->streams[0];
    assert(video->codecpar->codec_type == AVMEDIA_TYPE_VIDEO);

    const AVOutputFormat *mpegts = av_guess_format("mpegts", NULL, NULL);
    if(!mpegts)
        fprintf(stderr, "FFmpeg doesn't support mpegts\n");
    avformat_alloc_output_context2(&out, mpegts, NULL, STREAM_REMOTE);
    AVStream *out_video = avformat_new_stream(out, NULL);
    ret = avcodec_parameters_copy(out_video->codecpar, video->codecpar);
    if(ret < 0)
        fprintf(stderr, "avcodec_parameters_copy() failed\n");
    avio_open(&out->pb, STREAM_REMOTE, AVIO_FLAG_WRITE);
    ret = avformat_write_header(out, NULL);
    if(ret < 0)
        fprintf(stderr, "avformat_write_header() failed\n");
}

int stream_thread(void *arg)
{
    AVPacket *pkt = av_packet_alloc();
    AVStream *in_video = camera->streams[0],
             *out_video = out->streams[0];
    int ret;

    while(true) {
        ret = av_read_frame(camera, pkt);
        if(ret < 0) {
            if(ret == AVERROR(EAGAIN) ||
               ret == AVERROR_EOF)
                continue;
            else
                fprintf(stderr, "av_read_frame() failed\n");
        }
        pkt->stream_index = 0; // video stream in output
        pkt->pts = av_rescale_q_rnd(pkt->pts,
                                    in_video->time_base,
                                    out_video->time_base,
                                    AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        pkt->dts = av_rescale_q_rnd(pkt->dts,
                                    in_video->time_base,
                                    out_video->time_base,
                                    AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        pkt->duration = av_rescale_q(pkt->duration, in_video->time_base, out_video->time_base);
        pkt->pos = -1;
        ret = av_interleaved_write_frame(out, pkt);
    }

    av_write_trailer(out);
}
