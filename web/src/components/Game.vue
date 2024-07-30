<template>
  <div id="app">
    <video ref="remoteVideo" autoplay playsinline/>
  </div>
</template>
  
<script setup lang="ts">
import { ref, onMounted } from 'vue';

const configuration = {
  iceServers: [{
    urls: 'stun:stun.l.google.com:19302'
  }]
};

const remoteVideo = ref<HTMLVideoElement | null>(null);
const peerConnection = new RTCPeerConnection(configuration);
const signalingServer = new WebSocket('ws://localhost:8088');

function sendData(data: any) {
  if(signalingServer.readyState)
    signalingServer.send(JSON.stringify(data));
  else
    setTimeout(() => sendData(data), 1000);
}

const setupWebRTC = async () => {
  // Set up the onicecandidate event handler
  peerConnection.onicecandidate = (event) => {
    console.log("ice cand event", event);
    console.log("sending")
    // Send the candidate to the remote peer through the signaling server
    if (event.candidate)
      sendData({type: 'candidate', candidate: event.candidate});
  };

  // Function to handle received ICE candidates from the remote peer
  function handleRemoteICECandidate(candidate: any) {
    peerConnection.addIceCandidate(new RTCIceCandidate(candidate))
      .catch(e => console.error('Error adding received ICE candidate', e));
  }

  // Handle received offer and send an answer
  signalingServer.onmessage = (message) => {
    message = JSON.parse(message.data);
    if (message.type === 'offer') {
      // peerConnection.setRemoteDescription(new RTCSessionDescription(message.offer))
      //   .then(() => peerConnection.createAnswer())
      //   .then(answer => peerConnection.setLocalDescription(answer))
      //   .then(() => {
      //     sendData({
      //       type: 'answer',
      //       answer: peerConnection.localDescription,
      //     });
      //   })
      //   .catch(e => console.error('Error handling received offer', e));
    } else if (message.type === 'answer') {
        console.log("recv answer", message.sdp);
        peerConnection.setRemoteDescription({
          type: 'answer',
          sdp: message.sdp,
        })
          .then(() => {
            console.log('Remote description set');
          })
          .catch(e => console.error('Error handling received answer', e));
    } else if (message.type === 'candidate') {
      console.log("recv candidate", message.candidate);
      peerConnection.addIceCandidate({
        candidate: message.candidate,
        sdpMid: message.mid,
      })
        .catch(e => console.error('Error adding received ICE candidate', e));
    }
  };

  peerConnection.ontrack = (event) => {
    if (event.track.kind === 'video' && remoteVideo.value) {
      remoteVideo.value.srcObject = event.streams[0];
    }
  };

  const offerOptions = {
    offerToReceiveAudio: true,
    // offerToReceiveVideo: false,
    // iceRestart: false,
    // voiceActivityDetection: false,
  };

  // Create and send an offer to start the connection process
  const offer = await peerConnection.createOffer(offerOptions);
  await peerConnection.setLocalDescription(offer);
  // sendData({type: 'offer', offer: peerConnection.localDescription });
  sendData(peerConnection.localDescription);
  console.log('Offer sent', peerConnection.localDescription);
}

onMounted(async () => {
  setupWebRTC();
});
</script>
<style>
#remoteVideo {
  width: 100%;
  height: auto;
}
</style>
