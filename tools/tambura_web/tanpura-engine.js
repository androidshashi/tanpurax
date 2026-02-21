/**
 * TANPURA STUDIO REFERENCE ENGINE
 * -------------------------------
 * Focus: High Volume, Spectral Clarity, Continuous Flow
 */

class TanpuraEngine {
    constructor() {
        this.ctx = null;
        this.isPlaying = false;
        this.schedulerId = null;
        this.nextTime = 0.0;
        this.idx = 0;
        this.masterGain = null;
        this.analyser = null;
        
        // Settings provided by UI
        this.settings = {
            pitch: 146.83,
            firstStringRatio: 0.75,
            tempo: 1.0
        };

        // Callback for UI visualization
        this.onTriggerString = null;
    }

    async init() {
        const AudioContext = window.AudioContext || window.webkitAudioContext;
        if (!this.ctx) {
            this.ctx = new AudioContext();

            this.analyser = this.ctx.createAnalyser();
            this.analyser.fftSize = 256;

            const comp = this.ctx.createDynamicsCompressor();
            comp.threshold.value = -8;
            comp.ratio.value = 4;
            comp.attack.value = 0.01;
            comp.release.value = 0.1;

            this.masterGain = this.ctx.createGain();
            this.masterGain.gain.value = 0.8;

            this.masterGain.connect(comp);
            comp.connect(this.analyser);
            this.analyser.connect(this.ctx.destination);
        }

        if (this.ctx.state === 'suspended') {
            await this.ctx.resume();
        }
    }

    updateSettings(pitch, ratio, tempo) {
        this.settings.pitch = parseFloat(pitch);
        this.settings.firstStringRatio = parseFloat(ratio);
        this.settings.tempo = parseFloat(tempo);
    }

    schedule() {
        if (!this.isPlaying) return;

        while (this.nextTime < this.ctx.currentTime + 0.1) {
            this.playString(this.idx, this.nextTime);
            const gap = 1.3 / this.settings.tempo;
            this.nextTime += gap;
            this.idx = (this.idx + 1) % 4;
        }
        this.schedulerId = setTimeout(() => this.schedule(), 25);
    }

    playString(i, time) {
        const root = this.settings.pitch;
        const ratio = this.settings.firstStringRatio;

        let freq = root;
        let pan = 0;
        let gainMod = 1.0;
        let eqType = 'peaking';
        let eqFreq = 1000;
        let eqGain = 0;

        if (i === 0) {
            freq = root * ratio;
            pan = -0.6;
            gainMod = 0.9;
            eqType = 'highshelf';
            eqFreq = 3000;
            eqGain = 5;
        } else if (i === 1) {
            freq = root * 1.001;
            pan = -0.2;
        } else if (i === 2) {
            freq = root * 0.999;
            pan = 0.2;
        } else {
            freq = root * 0.5;
            pan = 0.6;
            gainMod = 1.2;
            eqType = 'lowshelf';
            eqFreq = 150;
            eqGain = 4;
        }

        const osc1 = this.ctx.createOscillator();
        const osc2 = this.ctx.createOscillator();
        osc1.type = 'sine';
        osc2.type = 'sawtooth';
        osc1.frequency.value = freq;
        osc2.frequency.value = freq;

        const javariFilter = this.ctx.createBiquadFilter();
        javariFilter.type = 'bandpass';
        javariFilter.Q.value = 3.0;

        const eqFilter = this.ctx.createBiquadFilter();
        eqFilter.type = eqType;
        eqFilter.frequency.value = eqFreq;
        eqFilter.gain.value = eqGain;

        const env = this.ctx.createGain();
        const duration = 12.0;

        env.gain.setValueAtTime(0, time);
        env.gain.linearRampToValueAtTime(0.5 * gainMod, time + 0.8);
        env.gain.exponentialRampToValueAtTime(0.001, time + duration);

        javariFilter.frequency.setValueAtTime(freq * 6, time);
        javariFilter.frequency.exponentialRampToValueAtTime(freq, time + (duration / 2));

        const panner = this.ctx.createStereoPanner();
        panner.pan.value = pan;

        osc1.connect(eqFilter);
        osc2.connect(javariFilter);
        javariFilter.connect(eqFilter);
        eqFilter.connect(env);
        env.connect(panner);
        panner.connect(this.masterGain);

        osc1.start(time);
        osc2.start(time);
        osc1.stop(time + duration);
        osc2.stop(time + duration);

        // Visual Sync Callback
        if (this.onTriggerString) {
            const delay = (time - this.ctx.currentTime) * 1000;
            setTimeout(() => this.onTriggerString(i), delay);
        }
    }

    start() {
        this.masterGain.gain.setValueAtTime(0.8, this.ctx.currentTime);
        this.isPlaying = true;
        this.nextTime = this.ctx.currentTime + 0.1;
        this.idx = 0;
        this.schedule();
    }

    stop() {
        this.isPlaying = false;
        clearTimeout(this.schedulerId);
        if (this.masterGain) {
            this.masterGain.gain.setTargetAtTime(0, this.ctx.currentTime, 0.3);
        }
        setTimeout(() => { if (this.ctx) this.ctx.suspend(); }, 400);
    }

    getRMS() {
        if (!this.analyser) return 0;
        const arr = new Uint8Array(this.analyser.frequencyBinCount);
        this.analyser.getByteTimeDomainData(arr);
        let sum = 0;
        for (let i = 0; i < arr.length; i++) {
            const v = (arr[i] - 128) / 128;
            sum += v * v;
        }
        return Math.sqrt(sum / arr.length);
    }
}