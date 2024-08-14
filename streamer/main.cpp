#include <thread>
#include <unistd.h>
#include <rtc/rtc.hpp>
#include <nlohmann/json.hpp>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include "joystick.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
typedef int SOCKET;
#endif

using namespace std;
using namespace rtc;
using namespace nlohmann;

#undef av_err2str
#define av_err2str(errnum) av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

static int in_line[4]{};
static int64_t frame_number = 0;
static uint64_t timestamp_start = 0;
static SwsContext* sws_ctx = nullptr;


uint64_t microsecondsSinceEpoch() {
    return chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

void convert_to_yuvframe(vector<uint8_t> &frame_data, AVFrame *out_frame) {
    if (!sws_ctx) {
        av_image_fill_linesizes(in_line, AV_PIX_FMT_BGRA, 1280);
        sws_ctx = sws_getContext(
            1280, 720, AV_PIX_FMT_BGRA,  // Source format
            1280, 720, AV_PIX_FMT_YUV420P,  // Destination format
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }

    uint8_t* in_data[4]{};
    av_image_fill_pointers(in_data, AV_PIX_FMT_BGRA, 720, frame_data.data(), in_line);
    sws_scale(sws_ctx, in_data, in_line, 0, 720, out_frame->data, out_frame->linesize);
    out_frame->pts = frame_number++ * AV_TIME_BASE;
}

void encodeAndSendFrame(AVCodecContext* codecContext, AVFrame* frame, const shared_ptr<Track> &videoTrack, const shared_ptr<RtcpSrReporter> &srReporter) {
    AVPacket* packet = av_packet_alloc();
    int ret = avcodec_send_frame(codecContext, frame);

    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            cerr << "Error encoding frame: " << av_err2str(ret) << endl;
            break;
        }

        const double elapsedSeconds = static_cast<double>(microsecondsSinceEpoch() - timestamp_start) / 1000000;
        uint32_t elapsedTimestamp = srReporter->rtpConfig->secondsToTimestamp(elapsedSeconds);
        srReporter->rtpConfig->timestamp = srReporter->rtpConfig->startTimestamp + elapsedTimestamp;

        auto reportElapsedTimestamp = srReporter->rtpConfig->timestamp - srReporter->lastReportedTimestamp();

        if (srReporter->rtpConfig->timestampToSeconds(reportElapsedTimestamp) >= 1)
            srReporter->setNeedsToReport();

        videoTrack->send(reinterpret_cast<byte*>(packet->data), packet->size * sizeof(byte));
        av_packet_unref(packet);
    }

    // cout << "Frame encoded and sent" << endl;
    av_packet_free(&packet);
}

WebSocket webSocket;
shared_ptr<Track> videoTrack = nullptr;
shared_ptr<DataChannel> dc = nullptr;
shared_ptr<PeerConnection> pc = nullptr;
shared_ptr<RtcpSrReporter> srReporter = nullptr;

void handle_offer(string offer_str) {
    json offer_json = json::parse(offer_str);
    Configuration config;
    config.iceServers.emplace_back("stun.l.google.com:19302");
    pc = make_shared<PeerConnection>(config);
    pc->onGatheringStateChange(
        [](PeerConnection::GatheringState state) {
            if (state == PeerConnection::GatheringState::Complete) {
                auto description = pc->localDescription();
                json message = {{"type", "answer"}, {"sdp", string(description.value())}};
                string answer = message.dump();
                cout << answer << endl;
                webSocket.send(answer);
            }
        }
    );

    const SSRC ssrc = 1;
    const string cname = "0";
    auto videoDesc = Description::Video(cname, Description::Direction::SendOnly);
    videoDesc.addH264Codec(106);
    videoDesc.addSSRC(ssrc, cname);
    videoTrack = pc->addTrack(videoDesc);

    auto rtpConfig = make_shared<RtpPacketizationConfig>(ssrc, cname, 106, H264RtpPacketizer::defaultClockRate);
    auto packetizer = make_shared<H264RtpPacketizer>(NalUnit::Separator::LongStartSequence, rtpConfig);
    srReporter = make_shared<RtcpSrReporter>(rtpConfig);
    packetizer->addToChain(srReporter);
    auto nackResponder = make_shared<RtcpNackResponder>();
    packetizer->addToChain(nackResponder);
    videoTrack->setMediaHandler(packetizer);

    pc->onDataChannel([](const shared_ptr<DataChannel> &channel) {
        dc = channel;
        create_joystick();

        dc->onMessage([](variant<binary, string> message) {
            if (holds_alternative<string>(message)) {
                cout << "DataChannel message: " << get<string>(message) << endl;
                json msg_json = json::parse(get<string>(message));
                for (auto &msg : msg_json) {
                    cout << "Type: " << msg["type"].get<int>() << endl;
                    cout << "Code: " << msg["code"].get<int>() << endl;
                    cout << "Value: " << msg["value"].get<int>() << endl;
                }
            }
        });
    });

    Description offer(offer_json["sdp"].get<string>(), Description::Type::Offer);
    pc->setRemoteDescription(offer);
}

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    freeResources();
    exit(signum);
}

void freeResources(){
    // av_freep(&frame->data[0]);
    // av_frame_free(&frame); // dont forget to free memory
    // avcodec_free_context(&codecContext);
    // sws_freeContext(sws_ctx);
    libevdev_uinput_destroy(uidev);
    libevdev_free(dev);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    InitLogger(LogLevel::Info);

    webSocket.onOpen([]() { cout << "WebSocket open" << endl; });
    webSocket.onMessage([](variant<binary, string> message) {
        if (holds_alternative<string>(message)) {
            auto msg_json = json::parse(get<string>(message));
            auto type = msg_json["type"].get<string>();
            if (type == "offer") handle_offer(get<string>(message));
        }
    });
    webSocket.open("ws://7.tcp.eu.ngrok.io:17135");

    avformat_network_init();

    const AVCodec* codec = avcodec_find_encoder_by_name("h264_nvenc");
    if (!codec) {
        cerr << "Codec 'h264_nvenc' not found" << endl;
        return -1;
    }

    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    codecContext->bit_rate = 1024000;
    codecContext->width = 1280;
    codecContext->height = 720;
    codecContext->time_base = (AVRational){1, 25};
    codecContext->framerate = (AVRational){25, 1};
    codecContext->gop_size = 10;
    codecContext->max_b_frames = 0;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    // av_opt_set(codecContext->priv_data, "profile", "baseline", 0);
    // av_opt_set(codecContext->priv_data, "level", "3.1", 0);
    // av_opt_set(codecContext->priv_data, "packetization-mode", "1", 0);

    avcodec_open2(codecContext, codec, nullptr);
    AVFrame* frame = av_frame_alloc();
    frame->format = codecContext->pix_fmt;
    frame->width = codecContext->width;
    frame->height = codecContext->height;
    av_image_alloc(
        frame->data, frame->linesize, codecContext->width, codecContext->height, codecContext->pix_fmt, 32
    );

    const auto *shmName = "/vectframe_shm";
    const int shm_fd = shm_open(shmName, O_RDONLY, 0666);
    if (shm_fd == -1) {
        cerr << "Failed to open shared memory: " << shmName << endl;
        return -1;
    }

    constexpr int SHM_SIZE = 1280 * 720 * 4;
    const auto shm_ptr = static_cast<uint8_t*>(mmap(nullptr, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0));
    if (shm_ptr == MAP_FAILED) {
        cerr << "Failed to map shared memory" << endl;
        close(shm_fd);
        return -1;
    }

    vector<uint8_t> curr_frame(SHM_SIZE, 0);
    while (!videoTrack || !videoTrack->isOpen())
        this_thread::sleep_for(chrono::milliseconds(500));

    timestamp_start = microsecondsSinceEpoch();

    while (true) {
        copy_n(shm_ptr, SHM_SIZE, curr_frame.begin());
        convert_to_yuvframe(curr_frame, frame);
        encodeAndSendFrame(codecContext, frame, videoTrack, srReporter);
        this_thread::sleep_for(chrono::milliseconds(10)); // At most 100 FPS
    }
}
