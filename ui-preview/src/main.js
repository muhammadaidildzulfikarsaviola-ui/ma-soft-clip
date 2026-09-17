import './style.css'

const state = {
  gain: 0,
  output: 0,
  threshold: 0,
  knee: 50,
  style: 0,
  mode: 0,
  oversampling: 0,
  signal: 1,
  clipper: true,
  inputDb: -60,
  outputDb: -60,
  grDb: 0,
}

const styleNames = ['SOFT', 'MEDIUM', 'HARD']
const modeNames = ['STEREO', 'M/S', 'MULTIBAND']
const osNames = ['1x', '2x', '4x', '8x']
const signalNames = ['IN', 'GR', 'OUT']
const embedded = () => !!window.__JUCE__?.backend

function emitNative(eventId, payload = {}) {
  if (embedded())
    window.__JUCE__.backend.emitEvent(eventId, payload)
}

const app = document.querySelector('#app')

app.innerHTML = `
  <main class="chassis">
    <header class="header">
      <div class="brand-left"><h1>SOFT CLIP</h1><span>CLEAN LOUDER TOGETHER</span></div>
      <div class="brand-right"><strong>WADIDAW</strong><small>AUDIO TOOLS</small></div>
    </header>

    <section class="panel meters-panel">
      <span class="screw s1"></span><span class="screw s2"></span><span class="screw s3"></span><span class="screw s4"></span>
      <div class="meter-pair"><div class="meter-stack"><div class="meter"><i id="inL"></i></div><div class="meter"><i id="inR"></i></div></div><output id="inputReadout">-60.0 dB<small>LEVEL IN</small></output></div>
      <div class="meter-pair out-pair"><div class="meter-stack"><div class="meter"><i id="outL"></i></div><div class="meter"><i id="outR"></i></div></div><output id="outputReadout">-60.0 dB<small>LEVEL OUT</small></output></div>
    </section>

    <section class="panel center-panel">
      <span class="screw s1"></span><span class="screw s2"></span><span class="s3 screw"></span><span class="s4 screw"></span>
      <div class="gr-display"><div class="grid"></div><div class="gr-title">GAIN REDUCTION</div><div class="gr-value" id="grValue">0.0 dB</div><div class="needle" id="needle"></div></div>
      <div class="top-knobs">
        <div class="control"><label>THRESHOLD</label><div class="knob" id="thresholdKnob"><span></span></div><output id="thresholdValue">0.0 dB</output></div>
        <div class="control"><label>KNEE</label><div class="knob" id="kneeKnob"><span></span></div><output id="kneeValue">50 %</output></div>
      </div>
      <div class="bottom-controls">
        <div class="control large"><label>GAIN</label><div class="knob" id="gainKnob"><span></span></div><output id="gainValue">+0.0 dB</output></div>
        <div class="signal-control"><label>SIGNAL</label><div class="signal-switch" id="signalSwitch"><span></span></div><output id="signalValue">GR</output></div>
        <div class="control large"><label>OUTPUT</label><div class="knob" id="outputKnob"><span></span></div><output id="outputValue">+0.0 dB</output></div>
      </div>
    </section>

    <section class="panel right-panel">
      <span class="screw s1"></span><span class="screw s2"></span><span class="screw s3"></span><span class="screw s4"></span>
      <div class="choice-control"><label>CLIPPER STYLE</label><div class="track" data-key="style"><span></span></div><output id="styleValue">SOFT</output></div>
      <div class="choice-control"><label>MODE</label><div class="track" data-key="mode"><span></span></div><output id="modeValue">STEREO</output></div>
      <div class="choice-control"><label>OVERSAMPLING</label><div class="track" data-key="oversampling"><span></span></div><output id="osValue">1x</output></div>
    </section>

    <section class="panel clipper-panel">
      <span class="screw s1"></span><span class="screw s2"></span><span class="screw s3 screw"></span><span class="screw s4 screw"></span>
      <button id="clipperToggle" class="toggle on"><span></span><b>CLIPPER</b><em>ON</em></button>
    </section>
    <footer><span>WADIDAW AUDIO TOOLS</span><strong>CLEAN LOUDER TOGETHER</strong><span>EST. 2025</span></footer>
  </main>
`

const $ = id => document.getElementById(id)

function setKnob(el, value, min, max, output, formatter) {
  const p = Math.max(0, Math.min(1, (value - min) / (max - min)))
  el.style.setProperty('--angle', `${-135 + p * 270}deg`)
  output.textContent = formatter(value)
}

function render() {
  setKnob($('gainKnob'), state.gain, -12, 12, $('gainValue'), v => `${v >= 0 ? '+' : ''}${v.toFixed(1)} dB`)
  setKnob($('outputKnob'), state.output, -12, 12, $('outputValue'), v => `${v >= 0 ? '+' : ''}${v.toFixed(1)} dB`)
  setKnob($('thresholdKnob'), state.threshold, -20, 0, $('thresholdValue'), v => `${v.toFixed(1)} dB`)
  setKnob($('kneeKnob'), state.knee, 0, 100, $('kneeValue'), v => `${Math.round(v)} %`)

  $('styleValue').textContent = styleNames[state.style] ?? 'SOFT'
  $('modeValue').textContent = modeNames[state.mode] ?? 'STEREO'
  $('osValue').textContent = osNames[state.oversampling] ?? '1x'
  $('signalValue').textContent = signalNames[state.signal] ?? 'GR'
  $('clipperToggle').classList.toggle('on', state.clipper)
  document.querySelector('#clipperToggle em').textContent = state.clipper ? 'ON' : 'OFF'

  const inputPercent = Math.max(0, Math.min(100, ((state.inputDb + 60) / 60) * 100))
  const outputPercent = Math.max(0, Math.min(100, ((state.outputDb + 60) / 60) * 100))
  ;['inL', 'inR'].forEach(id => $(id).style.height = `${inputPercent}%`)
  ;['outL', 'outR'].forEach(id => $(id).style.height = `${outputPercent}%`)
  $('inputReadout').firstChild.textContent = `${state.inputDb.toFixed(1)} dB`
  $('outputReadout').firstChild.textContent = `${state.outputDb.toFixed(1)} dB`
  $('grValue').textContent = `${state.grDb <= -0.01 ? state.grDb.toFixed(1) : '0.0'} dB`
  const grAmount = Math.max(0, Math.min(20, Math.abs(state.grDb)))
  $('needle').style.transform = `rotate(${-55 + grAmount * 5}deg)`
}

function setParameter(id, value) {
  emitNative('setParameter', { id, value })
}

function cycle(key, max, parameterId) {
  state[key] = (state[key] + 1) % max
  setParameter(parameterId, state[key])
  render()
}

$('gainKnob').onclick = () => { state.gain = state.gain >= 12 ? -12 : state.gain + 1; setParameter('input_gain', state.gain); render() }
$('outputKnob').onclick = () => { state.output = state.output >= 12 ? -12 : state.output + 1; setParameter('output_gain', state.output); render() }
$('thresholdKnob').onclick = () => { state.threshold = state.threshold >= 0 ? -20 : state.threshold + 1; setParameter('threshold', state.threshold); render() }
$('kneeKnob').onclick = () => { state.knee = state.knee >= 100 ? 0 : state.knee + 10; setParameter('knee', state.knee); render() }
$('signalSwitch').onclick = () => cycle('signal', 3, 'signal_select')
$('clipperToggle').onclick = () => { state.clipper = !state.clipper; setParameter('bypass', state.clipper ? 1 : 0); render() }
document.querySelector('[data-key="style"]').onclick = () => cycle('style', 3, 'clip_style')
document.querySelector('[data-key="mode"]').onclick = () => cycle('mode', 3, 'mode')
document.querySelector('[data-key="oversampling"]').onclick = () => cycle('oversampling', 4, 'oversampling')

window.setPluginState = next => { Object.assign(state, next); render() }
emitNative('uiReady')

function animatePreview() {
  if (embedded()) return
  const t = performance.now() / 1000
  state.inputDb = -60 + (34 + Math.abs(Math.sin(t * 2.2)) * 46) * 0.52
  state.outputDb = state.inputDb - 4
  state.grDb = -Math.max(0, state.inputDb - Math.max(-20, state.threshold)) * 0.35
  render()
  requestAnimationFrame(animatePreview)
}

render()
animatePreview()
