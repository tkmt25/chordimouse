const CONFIG_SERVICE_UUID = 0xF00D;
const GLOBAL_CONFIG_CHR_UUID = 0xFF01;
const KEYPROFILE_CONFIG_CHR_UUID = 0xFF02;

const Button = {
    B1: 0x0001,
    B2: 0x0002,
    B3: 0x0004,
    B4: 0x0008,
    MB1: 0x0010,
    JB: 0x0020,
    JU: 0x0040,
    JD: 0x0080,
    JL: 0x0100,
    JR: 0x0200,
}

class KeyProfile {
    constructor() {
        this.chordMap = {};
    }

    deserialize(dataview, offset=0) {
        this.chordMap = {};

        let _offset = offset;
        const recordCount = dataview.getUint16(_offset, true); _offset += 2;
            
        for (let i = 0; i < recordCount; i++) {
            const chord = dataview.getUint16(_offset, true); _offset += 2;
            const scancode = dataview.getUint8(_offset); _offset += 1;
            this.chordMap[chord] = scancode;
        }
        return _offset - offset;
    }

    serialize() {
        const keys = Object.keys(this.chordMap).map(k => parseInt(k));
        const size = 2 + ((2 + 1) * keys.length);
        const buff = new Uint8Array(size);
        const view = new DataView(buff.buffer);
        
        let offset = 0;
        view.setUint16(offset, Object.keys(this.chordMap).length, true);; offset += 2;
        for (const chord of keys) {
            const scancode = this.chordMap[chord];
            view.setUint16(offset, chord, true); offset += 2;
            view.setUint8(offset, scancode); offset += 1;
        }
        return buff;
    }
}

class ChordiMouse extends EventTarget{

    constructor() {
        super();
        this.device = null;
        this.server = null;
        this.service = null;
        this.globalChar = null;
        this.keyChar = null;
    }

    async connect() {
        this.device = await navigator.bluetooth.requestDevice({
            filters: [{ namePrefix: 'ChordiMouse' }],
            optionalServices: [CONFIG_SERVICE_UUID]
          });

        this.device.addEventListener('gattserverdisconnected', () => {
            this.dispatchEvent(new Event('disconnected'));
        });
        this.server = await this.device.gatt.connect();
        this.service = await this.server.getPrimaryService(CONFIG_SERVICE_UUID);
        this.globalChar = await this.service.getCharacteristic(GLOBAL_CONFIG_CHR_UUID);
        this.keyChar = await this.service.getCharacteristic(KEYPROFILE_CONFIG_CHR_UUID);
        this.dispatchEvent(new Event('connected'));
    }

    async disconnect() {
        await this.service.disconnect();
    }

    async isConnected() {
        return this.server?.isConnected ?? false;
    }

    async loadGlobalConfig() {
        const response = await this.globalChar.readValue();
        return JSON.parse( new TextDecoder().decode(response) );
    }

    async loadKeyProfiles() {
        const value = await this.keyChar.readValue();
        let offset = 0;
        const count = value.getUint8(offset); offset++;

        const profiles = [];
        for (let i = 0; i < count; i++) {
            const keyProfile = new KeyProfile();
            offset += keyProfile.deserialize(value, offset);
            profiles.push(keyProfile);
        }
        
        return profiles;
    }

    async saveGlobalConfig(config) {
        const encoder = new TextEncoder();
        const data = encoder.encode(JSON.stringify(config));
        await this.globalChar.writeValue(data);
    }

    async saveKeyProfiles(keyProfiles) {
        const profileData = keyProfiles.map(p => p.serialize());
        const totalSize = 1 + profileData.reduce((acc, buf) => acc + buf.length, 0); // +1 for count
        const buff = new Uint8Array(totalSize);
        const view = new DataView(buff.buffer);

        let offset = 0;
        view.setUint8(offset++, keyProfiles.length); // count is 1バイト

        for (const data of profileData) {
            for (let i = 0; i < data.length; i++) {
                view.setUint8(offset++, data[i]);
            }
        }
        await this.keyChar.writeValue(buff);
    }
}


const HID_KEYCODES = {
  0x00: "NONE",
  0x04: "A", 0x05: "B", 0x06: "C", 0x07: "D", 0x08: "E", 0x09: "F",
  0x0A: "G", 0x0B: "H", 0x0C: "I", 0x0D: "J", 0x0E: "K", 0x0F: "L",
  0x10: "M", 0x11: "N", 0x12: "O", 0x13: "P", 0x14: "Q", 0x15: "R",
  0x16: "S", 0x17: "T", 0x18: "U", 0x19: "V", 0x1A: "W", 0x1B: "X",
  0x1C: "Y", 0x1D: "Z",
  0x1E: "1", 0x1F: "2", 0x20: "3", 0x21: "4", 0x22: "5", 0x23: "6",
  0x24: "7", 0x25: "8", 0x26: "9", 0x27: "0",
  0x28: "ENTER", 0x29: "ESCAPE", 0x2A: "BACKSPACE", 0x2B: "TAB", 0x2C: "SPACE",
  0x2D: "MINUS", 0x2E: "EQUAL", 0x2F: "LEFT_BRACKET", 0x30: "RIGHT_BRACKET",
  0x31: "BACKSLASH", 0x32: "NON_US_HASH", 0x33: "SEMICOLON", 0x34: "APOSTROPHE",
  0x35: "GRAVE", 0x36: "COMMA", 0x37: "PERIOD", 0x38: "SLASH", 0x39: "CAPS_LOCK",
  0x3A: "F1", 0x3B: "F2", 0x3C: "F3", 0x3D: "F4", 0x3E: "F5", 0x3F: "F6",
  0x40: "F7", 0x41: "F8", 0x42: "F9", 0x43: "F10", 0x44: "F11", 0x45: "F12",
  0x46: "PRINT_SCREEN", 0x47: "SCROLL_LOCK", 0x48: "PAUSE",
  0x49: "INSERT", 0x4A: "HOME", 0x4B: "PAGE_UP", 0x4C: "DELETE",
  0x4D: "END", 0x4E: "PAGE_DOWN", 0x4F: "RIGHT_ARROW", 0x50: "LEFT_ARROW",
  0x51: "DOWN_ARROW", 0x52: "UP_ARROW",
  0x53: "NUM_LOCK", 0x54: "KEYPAD_DIVIDE", 0x55: "KEYPAD_MULTIPLY",
  0x56: "KEYPAD_SUBTRACT", 0x57: "KEYPAD_ADD", 0x58: "KEYPAD_ENTER",
  0x59: "KEYPAD_1", 0x5A: "KEYPAD_2", 0x5B: "KEYPAD_3",
  0x5C: "KEYPAD_4", 0x5D: "KEYPAD_5", 0x5E: "KEYPAD_6",
  0x5F: "KEYPAD_7", 0x60: "KEYPAD_8", 0x61: "KEYPAD_9", 0x62: "KEYPAD_0",
  0x63: "KEYPAD_DOT", 0x64: "NON_US_BACKSLASH",
  0x65: "APPLICATION", 0x66: "POWER", 0x67: "KEYPAD_EQUAL",
  0x68: "F13", 0x69: "F14", 0x6A: "F15", 0x6B: "F16",
  0x6C: "F17", 0x6D: "F18", 0x6E: "F19", 0x6F: "F20",
  0x70: "F21", 0x71: "F22", 0x72: "F23", 0x73: "F24",
  0x74: "EXECUTE", 0x75: "HELP", 0x76: "MENU", 0x77: "SELECT",
  0x78: "STOP", 0x79: "AGAIN", 0x7A: "UNDO", 0x7B: "CUT",
  0x7C: "COPY", 0x7D: "PASTE", 0x7E: "FIND", 0x7F: "MUTE",
  0x80: "VOLUME_UP", 0x81: "VOLUME_DOWN",
  0x82: "LOCKING_CAPS_LOCK", 0x83: "LOCKING_NUM_LOCK", 0x84: "LOCKING_SCROLL_LOCK",
  0x85: "KEYPAD_COMMA", 0x86: "KEYPAD_EQUAL_SIGN",
  0x87: "KANJI1", 0x88: "KANJI2", 0x89: "KANJI3", 0x8A: "KANJI4",
  0x8B: "KANJI5", 0x8C: "KANJI6", 0x8D: "KANJI7", 0x8E: "KANJI8", 0x8F: "KANJI9",
  0x90: "LANG1", 0x91: "LANG2", 0x92: "LANG3", 0x93: "LANG4", 0x94: "LANG5",
  0x95: "LANG6", 0x96: "LANG7", 0x97: "LANG8", 0x98: "LANG9",
  0x99: "ALTERNATE_ERASE", 0x9A: "SYSREQ", 0x9B: "CANCEL", 0x9C: "CLEAR",
  0x9D: "PRIOR", 0x9E: "RETURN", 0x9F: "SEPARATOR", 0xA0: "OUT",
  0xA1: "OPER", 0xA2: "CLEAR_AGAIN", 0xA3: "CRSEL", 0xA4: "EXSEL",
  0xE0: "LEFT_CONTROL", 0xE1: "LEFT_SHIFT", 0xE2: "LEFT_ALT", 0xE3: "LEFT_GUI",
  0xE4: "RIGHT_CONTROL", 0xE5: "RIGHT_SHIFT", 0xE6: "RIGHT_ALT", 0xE7: "RIGHT_GUI"
};

export {ChordiMouse, Button, HID_KEYCODES};