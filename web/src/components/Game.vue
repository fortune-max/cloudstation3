<template>
  <div id="app">
    <input type="text" id="message" placeholder="Signaling Server Address" />
    <button id="send">Send</button>
    <p>Video</p>
    <video ref="remoteVideo" autoplay playsinline/>
    <p>Offer</p>
    <pre id="offer-sdp"></pre>
    <p>Answer</p>
    <pre id="answer-sdp"></pre>
  </div>
</template>
  
<script setup lang="js">
import { ref, onMounted } from 'vue';

const configuration = {
  iceServers: [{
    urls: 'stun:stun.l.google.com:19302'
  }]
};

const remoteVideo = ref<HTMLVideoElement | null>(null);
const peerConnection = new RTCPeerConnection(configuration);
const signalingServer = new WebSocket('ws://localhost:8088');

signalingServer.onmessage = async (evt) => {
  if (typeof evt.data !== 'string') return;
  const message = JSON.parse(evt.data);
  if (message.type == "answer") {
    document.getElementById('answer-sdp').textContent = message.sdp;
    peerConnection.setRemoteDescription({ type: 'answer', sdp: message.sdp });
  }
}

function sendData(data) {
  if(signalingServer.readyState)
    signalingServer.send(JSON.stringify(data));
  else
    setTimeout(() => sendData(data), 1000);
}

const candidates = [];
let ansReceived = false;

const setupWebRTC = async () => {
  // Set up the onicecandidate event handler
  peerConnection.onicecandidate = (event) => {
    console.log("ice cand event", event);
    console.log("sending")
    // Send the candidate to the remote peer through the signaling server
    if (event.candidate)
      sendData({type: 'candidate', candidate: event.candidate});
  };

  // Handle received offer and send an answer
  signalingServer.onmessage = async (message) => {
    message = JSON.parse(message.data);
    if (message.type === 'answer') {
        console.log("recv answer", message.sdp);
        await peerConnection.setRemoteDescription({
          type: 'answer',
          sdp: message.sdp,
        });
        console.log('Answer received', message.sdp);
        ansReceived = true;
        console.log("adding candidates", candidates);
        candidates.forEach((candidate) => {
          peerConnection.addIceCandidate({
            candidate: candidate.candidate,
            sdpMid: candidate.mid,
          });
        });
    } else if (message.type === 'candidate') {
      console.log("message DBG", message);
      console.log("recv candidate", message.candidate);
      if (!ansReceived) 
        candidates.push(message);
      else
        peerConnection.addIceCandidate({
          candidate: message.candidate,
          sdpMid: message.mid,
        });
    } else {
      console.error('Unrecognized message type:', message.type);
    }
  };

  const canvas = document.createElement('canvas');
  const context = canvas.getContext('2d');
  const videoElement = document.getElementById('remoteVideo');

//   async function processRtpPackets(receiver) {
//     console.log("processRtpPackets", receiver);
//     const sender = receiver.track;
//     if (!sender) return;

//     const transformer = new TransformStream({
//         start() {
//             console.log('RTP packet processing started');
//         },
//         async transform(chunk, controller) {
//             // Inspect the incoming RTP packet
//             const { header, payload } = chunk;
//             console.log('RTP header:', header);
//             console.log('RTP payload:', payload);

//             // Forward the packet to the next stage
//             controller.enqueue(chunk);
//         },
//         flush() {
//             console.log('RTP packet processing finished');
//         }
//     });

//     // Apply the transformer to the receiver
//     const { readable, writable } = transformer;
//     receiver.transform.readable.pipeTo(writable);
// }

  // navigator.mediaDevices.getUserMedia({ video: true })
  //   .then((stream) => {
  //       console.log("stream", stream);
  //       const videoElement = document.getElementById('remoteVideo');
  //       videoElement.srcObject = stream;
  //       videoElement?.addEventListener('loadedmetadata', () => {
  //         console.log('Remote video videoWidth: ', videoElement.videoWidth,
  //                     'px,  videoHeight: ', videoElement.videoHeight, 'px');
  //       });
  //       videoElement?.addEventListener('play', () => {
  //         console.log('Remote video started');
  //         drawFrame();
  //       });
  //   })
  //   .catch((error) => {
  //       console.error('Error accessing local media:', error);
  //   });

  peerConnection.ontrack = (event) => {
    console.log('Track event', event);
    if (event.track.kind === 'video') {
      console.log('Setting remote video stream');
      videoElement.srcObject = event.streams[0];
      // videoElement.play();
      // processRtpPackets(event.receiver);
      videoElement?.addEventListener('play', () => {
        console.log('Remote video started');
      });
    } else {
      console.error('Unrecognized track kind:', event.track.kind);
    }
  };

  const offerOptions = {
    // offerToReceiveAudio: false,
    offerToReceiveVideo: true,
    // iceRestart: false,
    // voiceActivityDetection: false,
  };

  // Create and send an offer to start the connection process
  const offer = await peerConnection.createOffer(offerOptions);
  await peerConnection.setLocalDescription(offer);
  async function waitGatheringComplete() {
    return new Promise((resolve) => {
        if (peerConnection.iceGatheringState === 'complete') {
            resolve(void 0);
        } else {
          peerConnection.addEventListener('icegatheringstatechange', () => {
                if (peerConnection.iceGatheringState === 'complete') {
                    resolve(void 0);
                }
            });
        }
    });
  }
  // sendData({type: 'offer', offer: peerConnection.localDescription });
  await waitGatheringComplete();
  sendData(peerConnection.localDescription);
  console.log('Offer sent', peerConnection.localDescription);
}

onMounted(async () => {
  setupWebRTC();
});
</script>
<style>
#app {
  height: 100vh;
  background-color: white;
  color: black;
}

#remoteVideo {
  width: 100%;
  height: auto;
}
</style>
