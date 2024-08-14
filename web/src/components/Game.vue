<style lang="scss">
#wrapper {
  width: 100%;
  display: flex;
  gap: 30px;
  justify-content: center;
  align-items: center;
  flex-direction: column;
  margin-top: 20px;
}
.video-section {
  display: flex;
  flex-direction: column;
  gap: 20px;
}
.signal {
  display: flex;
  gap: 10px;
  flex-direction: column;
  align-items: center;

  .v-input__details {
    display: none !important;
  }
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
}
#remoteVideo {
  width: 100%;
  height: 350px;
}
.form {
  display: flex;
  gap: 15px;
}
</style>
<template>
  <div id="wrapper">
    <div class="video-section">
      <h2>FPS: {{ fps }}</h2>
      <video id="remoteVideo" autoplay controls></video>
    </div>
    <div class="signal">
      <div class="form">
        <v-text-field
          v-model="textBoxValue"
          :label="textboxLabel"
          variant="outlined"
          width="300"
          :disabled="!textboxEnabled"
        />
        <v-btn
          color="light-blue"
          width="160"
          height="55"
          @click="makeOffer"
          :disabled="btnDisabled"
        >
          {{ btnText }}
        </v-btn>
        <v-snackbar v-model="showToast" :timeout="3000">
          {{ toastText }}
        </v-snackbar>
      </div>
      <v-switch
        v-model="useWebSockets"
        color="primary"
        label="Use Signaling Server"
      />
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
        <p>Connected: 🎮 {{ gamepadIdx !== -1 ? '✅' : '❌' }}</p>
        <p>Input Events:</p>
        <ul class="ml-10">
          <li v-for="event in recentEvents" :key="event">
            {{ event }}
          </li>
        </ul>
      </div>
    </div>
  </div>
</template>
  
<script setup lang="js">
import { ref, onMounted, watch, computed } from 'vue';
import gamepadCfgRaw from './ps5_gamepad.cfg.json';
import { gamepadDiff, transformBtn, transformAxis } from '../utils.ts';

const wsAddr = 'ws://7.tcp.eu.ngrok.io:17135';
const textBoxValue = ref(wsAddr);
const gamepadConnected = ref(true);
const events = ref({});
const fps = ref(30);
const useWebSockets = ref(true);
const showToast = ref(false);
const toastText = ref('');
const signalingServerConnected = ref(false);

const iceConnectionState = ref('');
const iceGatheringState = ref('');
const signalingState = ref('');

let peerConnection;
let signalingServer;
let dataChannel;

const btnText = computed(() => {
  if (useWebSockets.value) return 'Start!';
  if (signalingState.value.endsWith('have-local-offer')) return 'Submit Answer!';
  return 'Copy Offer!';
});

const btnDisabled = computed(() => {
  if (iceConnectionState.value.endsWith('connected')) return true;
  if (btnText.value === 'Submit Answer!' && !textBoxValue.value) return true;
  return useWebSockets.value && (!textBoxValue.value || !signalingServerConnected.value);
});

const textboxEnabled = computed(() => {
  if (useWebSockets.value) return true;
  if (signalingState.value.endsWith('have-local-offer')) return true;
  return false;
});

const textboxLabel = computed(() => {
  if (useWebSockets.value) return 'Signaling Server';
  if (signalingState.value.endsWith('have-local-offer')) return 'Answer';
  return '';
});

const connectToSignalingServer = async (address) => {
  if (signalingServer && signalingServer.readyState === 1)
    signalingServer.close();
  signalingServerConnected.value = false;
  signalingServer = new WebSocket(address);
  signalingServer.onopen = () => { signalingServerConnected.value = true; };
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
  if (!useWebSockets.value && signalingState.value.endsWith('have-local-offer')) {
    peerConnection.setRemoteDescription(JSON.parse(textBoxValue.value));
    toastText.value = 'Answer Submitted!';
    showToast.value = true;
    return;
  }
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

  dataChannel = peerConnection.createDataChannel('gamepad');

  const offer = await peerConnection.createOffer({
    offerToReceiveVideo: true,
  });
  await peerConnection.setLocalDescription(offer);
  await waitGatheringComplete();
  if (useWebSockets.value)
    sendData(peerConnection.localDescription);
  else {
    navigator.clipboard.writeText(JSON.stringify(peerConnection.localDescription));
    toastText.value = 'Offer copied to clipboard!';
    showToast.value = true;
    globalThis.peerConnection = peerConnection;
  }
  startFPSCounter();
}

watch(() => textBoxValue.value, (newVal) => {
  if (useWebSockets.value) connectToSignalingServer(newVal);
});

watch(() => useWebSockets.value, (newVal) => {
  if (newVal) {
    textBoxValue.value = wsAddr;
    connectToSignalingServer(wsAddr);
  } else {
    textBoxValue.value = '';
  }
});

function sendData(data) {
  if(signalingServer.readyState)
    signalingServer.send(JSON.stringify(data));
  else
    setTimeout(() => sendData(data), 1000);
}

let oldState = null;
const gamepadIdx = ref(-1);
const configName = "defaultCfg";
const gamepadMap = gamepadCfgRaw[configName];
const btnMap = gamepadMap.button;
const axesMap = gamepadMap.axis;

const recentEvents = computed(() => {
  return Object.keys(events.value).filter((key) => {
    return new Date().getTime() - events.value[key] < 500;
  });
});

window.addEventListener("gamepadconnected", (e) => {
  gamepadIdx.value = e.gamepad.index;
});

window.addEventListener("gamepaddisconnected", () => {
  gamepadIdx.value = -1;
});

function currentGamepad() {
  if (gamepadIdx.value === -1) return null;
  return navigator.getGamepads()[gamepadIdx.value];
}

function animate() {
  const newState = currentGamepad();
  const diff = gamepadDiff(oldState, newState);
  if (diff && newState && diff.axes.length + diff.buttons.length){
    const chunk = [];
    diff.buttons.forEach((buttonIdx) => {
      events.value[btnMap[buttonIdx].name] = new Date().getTime();
      chunk.push(transformBtn(newState.buttons[buttonIdx].value, btnMap[buttonIdx].in, btnMap[buttonIdx].out));
    });
    diff.axes.forEach((axisIdx) => {
      events.value[axesMap[axisIdx].name] = new Date().getTime();
      chunk.push(transformAxis(newState.axes[axisIdx], axesMap[axisIdx].in, axesMap[axisIdx].out));
    });
    if (chunk.length){
      console.log('Sent', chunk.length, 'events')
      if (dataChannel?.readyState === 'open')
        dataChannel.send(JSON.stringify(chunk));
    };
  }
  oldState = newState;
  requestAnimationFrame(animate);
}

onMounted(async () => {
  requestAnimationFrame(animate);
  connectToSignalingServer(textBoxValue.value);
});
</script>
