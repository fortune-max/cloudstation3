<style>
#wrapper {
  width: 100%;
  display: flex;
  gap: 30px;
  justify-content: center;
  align-items: center;
  flex-direction: column;
  margin-top: 50px;
}
.video-section {
  display: flex;
  flex-direction: column;
  gap: 20px;
}
.state-gamepad {
  display: flex;
  gap: 30px;
}
.state {
  width: 400px;
}
.gamepad {
  width: 400px;
  display: flex;
  flex-direction: column;
  justify-content: center;
}
#remoteVideo {
  width: 100%;
  height: 400px;
}
</style>
<template>
  <div id="wrapper">
    <div class="video-section">
      <h2>FPS: {{ fps }}</h2>
      <video id="remoteVideo" autoplay controls></video>
    </div>
    <div>
      <v-form>
        <v-text-field
          v-model="signalServerAddress"
          label="Signaling Server"
          variant="outlined"
          width="300"
        />
        <v-btn
          color="light-blue"
          width="300"
          height="50"
          @click="makeOffer"
          :disabled="!signalServerAddress || !signalingServerConnected"
        >
          Start!
        </v-btn>
      </v-form>
    </div>
    <div class="state-gamepad">
      <div class="state">
        <h2>State</h2>
        <p>ICE Connection state: {{ iceConnectionState }}</p>
        <p>ICE Gathering state: {{ iceGatheringState }}</p>
        <p>Signaling state: {{ signalingState }}</p>
      </div>
      <div class="gamepad">
        <h2>Gamepad</h2>
        <p>Connected: 🎮 {{ gamepadConnected ? '✅' : '❌' }}</p>
        <p>Input Events:</p>
        <ul class="ml-10">
          <li v-for="button in buttons" :key="button">
            {{ button }}
          </li>
        </ul>
      </div>
    </div>
  </div>
</template>
  
<script setup lang="js">
import { ref, onMounted, watch } from 'vue';

const signalServerAddress = ref('ws://0.tcp.eu.ngrok.io:15844');
const gamepadConnected = ref(true);
const buttons = ref(["lol", "lol2"]);
const fps = ref(30);
const signalingServerConnected = ref(false);

const iceConnectionState = ref('');
const iceGatheringState = ref('');
const signalingState = ref('');

let peerConnection;
let signalingServer;

const connectToSignalingServer = async (address) => {
  if (signalingServer) signalingServer.close();
  signalingServerConnected.value = false;
  signalingServer = new WebSocket(address);
  signalingServer.onopen = () => { signalingServerConnected.value = true; };
  signalingServer.onclose = () => { signalingServerConnected.value = false; };
  signalingServer.onmessage = async (evt) => {
    if (typeof evt.data !== 'string') return;
    const message = JSON.parse(evt.data);
    if (message.type == "answer") {
      if (peerConnection) // handle this?
        peerConnection.setRemoteDescription({ type: 'answer', sdp: message.sdp });
    }
  };
};

async function waitGatheringComplete() {
  return new Promise((resolve) => {
    if (peerConnection.iceGatheringState === 'complete') {
      resolve();
    } else {
      peerConnection.addEventListener('icegatheringstatechange', () => {
        if (peerConnection.iceGatheringState === 'complete')
          resolve();
      });
    }
  });
}

const startFPSCounter = () => {
  setInterval(async () => {
    const stats = await peerConnection.getStats();
    stats.forEach(report => {
      if (report.type === 'inbound-rtp' && report.kind === 'video' && report.framesPerSecond)
        fps.value = report.framesPerSecond;
    });
  }, 1000);
};

async function makeOffer() {
  const configuration = {iceServers: [{urls: 'stun:stun.l.google.com:19302'}]};
  peerConnection = new RTCPeerConnection(configuration);

  iceConnectionState.value = peerConnection.iceConnectionState;
  iceGatheringState.value = peerConnection.iceGatheringState;
  signalingState.value = peerConnection.signalingState;

  peerConnection.oniceconnectionstatechange = () => {
    iceConnectionState.value += ` -> ${peerConnection.iceConnectionState}`;
  };
  peerConnection.onicegatheringstatechange = () => {
    iceGatheringState.value += ` -> ${peerConnection.iceGatheringState}`;
  };
  peerConnection.onsignalingstatechange = () => {
    signalingState.value += ` -> ${peerConnection.signalingState}`;
  };

  peerConnection.ontrack = (evt) => {
    const video = document.getElementById('remoteVideo');
    video.srcObject = evt.streams[0];
    video.play();
  };

  const offer = await peerConnection.createOffer({
    offerToReceiveVideo: true,
  });
  await peerConnection.setLocalDescription(offer);
  await waitGatheringComplete();
  sendData(peerConnection.localDescription);
  startFPSCounter();
}

watch(() => signalServerAddress.value, connectToSignalingServer);

function sendData(data) {
  if(signalingServer.readyState)
    signalingServer.send(JSON.stringify(data));
  else
    setTimeout(() => sendData(data), 1000);
}

onMounted(async () => {
  connectToSignalingServer(signalServerAddress.value);
});
</script>
