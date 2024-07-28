#include <thread>
#include <unistd.h>
#include <rtc/rtc.hpp>
#include <nlohmann/json.hpp>
#include <sys/fcntl.h>
#include <sys/mman.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

using namespace std;
using namespace rtc;
using namespace nlohmann;

#undef av_err2str
#define av_err2str(errnum) av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

void lol(vector<uint8_t> &frame_data, AVFrame *out_frame) {
    uint8_t* in_data[4]{};
    int in_line[4]{};

    av_image_fill_linesizes(in_line, AV_PIX_FMT_BGRA, 1280);
    av_image_fill_pointers(in_data, AV_PIX_FMT_BGRA, 720, frame_data.data(), in_line);

    // Initialize the SwsContext for conversion
    SwsContext* sws_ctx = sws_getContext(
        1280, 720, AV_PIX_FMT_BGRA,  // Source format
        1280, 720, AV_PIX_FMT_YUV420P,  // Destination format
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    int res = sws_scale(
        sws_ctx, in_data, in_line,
        0, 720,
        out_frame->data, out_frame->linesize
    );
    if (res < 0) {
        cerr << "Error scaling-converting: " << av_err2str(res) << endl;
        return;
    }
}

void encodeAndSendFrame(AVCodecContext* codecContext, AVFrame* frame) { //, std::shared_ptr<rtc::Track> &videoTrack) {
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "Could not allocate AVPacket" << std::endl;
        return;
    }

    int ret = avcodec_send_frame(codecContext, frame);
    if (ret < 0) {
        std::cerr << "Error sending frame to encoder: " << av_err2str(ret) << std::endl;
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            std::cerr << "Error encoding frame: " << av_err2str(ret) << std::endl;
            break;
        }

        // videoTrack->send(reinterpret_cast<std::byte*>(packet->data), packet->size);
        av_packet_unref(packet);
    }
}

int main() {
    // InitLogger(LogLevel::Info);
    //
    // WebSocket ws;
    // Configuration config;
    // config.iceServers.emplace_back("stun.l.google.com:19302");
    // shared_ptr<PeerConnection> pc;
    // // auto pc = std::make_shared<rtc::PeerConnection>(config);
    //
    // ws.onOpen([]() { cout << "WebSocket open" << endl; });
    //
    // ws.onMessage([&config, &pc, &ws](variant<binary, string> message) {
    //     if (holds_alternative<string>(message)) {
    //         auto msg_json = json::parse(get<string>(message));
    //         auto type = msg_json["type"].get<string>();
    //         cout << "type: " << type << endl;
    //
    //         if (type == "offer") {
    //             pc = make_shared<PeerConnection>(config);
    //             pc->onLocalDescription([&ws](const Description& description) {
		  //           json message = {
		  //               {"type", "answer"},
		  //               {"sdp", string(description)}
    //                 };
    //                 ws.send(message.dump());
	   //          });
    //             pc->onLocalCandidate([&ws](const Candidate& candidate) {
    //                 json message = {
    //                     {"type", "candidate"},
    //                     {"candidate", string(candidate)},
    //                     {"mid", candidate.mid()}
    //                 };
    //                 ws.send(message.dump());
    //             });
    //             pc->onStateChange([](PeerConnection::State state) {
    //                 cout << "State: " << state << endl;
    //             });
    //             pc->onGatheringStateChange([](PeerConnection::GatheringState state) {
		  //           cout << "Gathering State: " << state << endl;
	   //          });
    //             pc->onTrack([](const shared_ptr<Track>& track) {
    //                 cout << "Track: " << track->mid() << endl;
    //             });
    //
    //             Description offer(msg_json["sdp"].get<string>(), Description::Type::Offer);
    //             pc->setRemoteDescription(offer);
    //         } else if (type == "candidate") {
    //             json candidate_json = msg_json["candidate"];
    //             Candidate candidate(candidate_json["candidate"].get<string>(), candidate_json["sdpMid"].get<string>());
    //             if (candidate.family() != Candidate::Family::Unresolved) {
    //                 cout << "Remote Candidate: " << candidate.candidate() << endl;
    //                 pc->addRemoteCandidate(candidate);
    //             }
    //         } else {
    //             cerr << "[Type not handled]: " << type << endl;
    //         }
    //     } else {
    //         cerr << "[WS message not handled]: " << get<binary>(message).size() << endl;
    //     }
    // });
    //
    // ws.open("ws://localhost:8088");

    // while (true) {
    //     this_thread::sleep_for(chrono::seconds(1));
    // }

    avformat_network_init();

    const AVCodec* codec = avcodec_find_encoder_by_name("h264_nvenc");
    if (!codec) {
        cerr << "Codec 'h264_nvenc' not found" << endl;
        return -1;
    }

    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        cerr << "Could not allocate video codec context" << endl;
        return -1;
    }

    codecContext->bit_rate = 512000;
    codecContext->width = 1280;
    codecContext->height = 720;
    codecContext->time_base = (AVRational){1, 25};
    codecContext->framerate = (AVRational){25, 1};
    codecContext->gop_size = 10;
    codecContext->max_b_frames = 1;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return -1;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        cerr << "Could not allocate video frame" << endl;
        return -1;
    }
    frame->format = codecContext->pix_fmt;
    frame->width = codecContext->width;
    frame->height = codecContext->height;

    av_image_alloc(
        frame->data, frame->linesize, codecContext->width,
        codecContext->height, codecContext->pix_fmt, 32
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
    while (true) {
        for (int j=0; j<SHM_SIZE; j++)
            curr_frame[j] = shm_ptr[j];
        lol(curr_frame, frame);

        encodeAndSendFrame(codecContext, frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(40)); // Simulate 25 FPS
    }


    av_freep(&frame->data[0]);
    av_frame_free(&frame); // dont forget to free memory
    avcodec_free_context(&codecContext);
}
