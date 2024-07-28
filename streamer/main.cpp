#include <rtc/rtc.hpp>
#include <nlohmann/json.hpp>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

using namespace std;
using namespace rtc;
using namespace nlohmann;

int main() {
    InitLogger(LogLevel::Info);

    WebSocket ws;
    Configuration config;
    config.iceServers.emplace_back("stun.l.google.com:19302");
    shared_ptr<PeerConnection> pc;
    // auto pc = std::make_shared<rtc::PeerConnection>(config);

    ws.onOpen([]() { cout << "WebSocket open" << endl; });

    ws.onMessage([&config, &pc, &ws](variant<binary, string> message) {
        if (holds_alternative<string>(message)) {
            auto msg_json = json::parse(get<string>(message));
            auto type = msg_json["type"].get<string>();
            cout << "type: " << type << endl;

            if (type == "offer") {
                pc = make_shared<PeerConnection>(config);
                pc->onLocalDescription([&ws](const Description& description) {
		            json message = {
		                {"type", "answer"},
		                {"sdp", string(description)}
                    };
                    ws.send(message.dump());
	            });
                pc->onLocalCandidate([&ws](const Candidate& candidate) {
                    json message = {
                        {"type", "candidate"},
                        {"candidate", string(candidate)},
                        {"mid", candidate.mid()}
                    };
                    ws.send(message.dump());
                });
                pc->onStateChange([](PeerConnection::State state) {
                    cout << "State: " << state << endl;
                });
                pc->onGatheringStateChange([](PeerConnection::GatheringState state) {
		            cout << "Gathering State: " << state << endl;
	            });
                pc->onTrack([](const shared_ptr<Track>& track) {
                    cout << "Track: " << track->mid() << endl;
                });

                Description offer(msg_json["sdp"].get<string>(), Description::Type::Offer);
                pc->setRemoteDescription(offer);
            } else if (type == "candidate") {
                json candidate_json = msg_json["candidate"];
                Candidate candidate(candidate_json["candidate"].get<string>(), candidate_json["sdpMid"].get<string>());
                if (candidate.family() != Candidate::Family::Unresolved) {
                    cout << "Remote Candidate: " << candidate.candidate() << endl;
                    pc->addRemoteCandidate(candidate);
                }
            } else {
                cerr << "[Type not handled]: " << type << endl;
            }
        } else {
            cerr << "[WS message not handled]: " << get<binary>(message).size() << endl;
        }
    });

    ws.open("ws://localhost:8088");

    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
    }
}
