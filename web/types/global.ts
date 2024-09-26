export type BrowserGamepadButton = {
    type: "button",
    initial?: number, // default 0
    max?: number, // default 1
    min?: number, // default 0
}

export type BrowserGamepadAxis = {
    type: "axis",
    initial?: number, // default 0
    max?: number, // default 1
    min?: number, // default -1
}

export type PCGamepadButton = {
    type: 1, // EV_KEY
    code: number, // e.g. BTN_SOUTH (304), ABS_X (0), REL_X (0)
    initial?: number, // 0 - released
    min?: number, // 0 - released
    max?: number, // 1 - pressed
}

export type PCGamepadAxis = {
    type: 3, // EV_ABS
    code: number,
    initial?: number, // 0 - released
    min?: number, // -32768 - released
    max?: number, // 32767 - pressed
}

export type Input = BrowserGamepadButton | BrowserGamepadAxis
export type Output = PCGamepadButton | PCGamepadAxis

export type InputMap = {
    name: string,
    in: Input,
    out: Output,
}

export type GamepadConfig = {
    button: Record<number, InputMap>,
    axis: Record<number, InputMap>,
}

export type GamepadConfigMap = Record<string, GamepadConfig>

export type EvdevEvent = {
    type: number,
    code: number,
    value: number,
}
