import type { Input, Output, EvdevEvent } from '../types/global.ts';

export function transformBtn(current: number, input: Input, output: Output){
    if (output.type === 1)
        return transformBtnToBtn(current, input, output);
    return transformBtnToAxis(current, input, output);
}

export function transformAxis(current: number, input: Input, output: Output){
    if (output.type === 1)
        return transformAxisToBtn(current, input, output);
    return transformAxisToAxis(current, input, output);
}

export function transformBtnToBtn(current: number, input: Input, output: Output): EvdevEvent{
    const percent = getPercentBtn(current, input.min, input.max, input.initial);
    const value = getValueBtn(percent, output.min, output.max, output.initial);
    return {
        type: output.type,
        code: output.code,
        value: Math.round(value),
    };
}

export function transformBtnToAxis(current: number, input: Input, output: Output): EvdevEvent{
    const percent = getPercentBtn(current, input.min, input.max, input.initial);
    const value = getValueAxis(percent, output.min, output.max, output.initial);
    return {
        type: output.type,
        code: output.code,
        value: Math.round(value),
    };
}

export function transformAxisToBtn(current: number, input: Input, output: Output): EvdevEvent{
    const percent = getPercentAxis(current, input.min, input.max, input.initial);
    const value = getValueBtn(percent, output.min, output.max, output.initial);
    return {
        type: output.type,
        code: output.code,
        value: Math.round(value),
    };
}

export function transformAxisToAxis(current: number, input: Input, output: Output): EvdevEvent{
    const percent = getPercentAxis(current, input.min, input.max, input.initial);
    const value = getValueAxis(percent, output.min, output.max, output.initial);
    return {
        type: output.type,
        code: output.code,
        value: Math.round(value),
    };
}

export function getPercentBtn(current: number, min: number = 0, max: number = 1, initial: number = 0){
    const range = max - min;
    const value = current - initial;
    const percent = value / range;
    return percent;
}

export function getValueBtn(percent: number, min: number = 0, max: number = 1, initial: number = 0){
    const range = max - min;
    const value = range * percent + initial;
    return value;
}

export function getPercentAxis(current: number, min: number = -32768, max: number = 32767, initial: number = 0){
    const range = max - min;
    const value = current - initial;
    const percent = value / range;
    return percent;
}

export function getValueAxis(percent: number, min: number = -32768, max: number = 32767, initial: number = 0){
    const range = max - min;
    const value = range * percent + initial;
    return value;
}

export function gamepadDiff(oldState: Gamepad | null, newState: Gamepad | null){
    if (!oldState || !newState) return null;
    const diff = {
        buttons: [] as number[],
        axes: [] as number[],
    };
    for (let i = 0; i < oldState.buttons.length; i++){
        if (oldState.buttons[i].pressed !== newState.buttons[i].pressed)
            diff.buttons.push(i);
    }
    for (let i = 0; i < oldState.axes.length; i++){
        if (oldState.axes[i] !== newState.axes[i])
            diff.axes.push(i);
    }
    return diff;
}
