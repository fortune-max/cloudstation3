<template>
  <span>
    Gamepad Index: {{ gamepadIdx }}
  </span>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import gamepadCfgRaw from './gamepad.cfg.json';
import { gamepadDiff, transformBtn, transformAxis } from '../utils.ts';
import type { GamepadConfig, EvdevEvent } from '../../types/global.ts';

const gamepadIdx = ref(-1);
const configName = "defaultCfg";
const gamepadMap = gamepadCfgRaw[configName] as GamepadConfig;
const btnMap = gamepadMap.button;
const axesMap = gamepadMap.axis;

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

let oldState = currentGamepad();

function animate() {
  const newState = currentGamepad();
  const diff = gamepadDiff(oldState, newState);
  if (diff && newState && diff.axes.length + diff.buttons.length){
    const chunk = [] as EvdevEvent[];
    diff.buttons.forEach((buttonIdx) => {
      chunk.push(transformBtn(newState.buttons[buttonIdx].value, btnMap[buttonIdx].in, btnMap[buttonIdx].out));
    });
    diff.axes.forEach((axisIdx) => {
      chunk.push(transformAxis(newState.axes[axisIdx], axesMap[axisIdx].in, axesMap[axisIdx].out));
    });
    if (chunk.length){
      // send chunk to server
      console.log('Chunk', chunk)
    };
  }
  oldState = newState;
  requestAnimationFrame(animate);
}

requestAnimationFrame(animate);
</script>
