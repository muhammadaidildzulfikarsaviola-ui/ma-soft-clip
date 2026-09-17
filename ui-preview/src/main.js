import './style.css'

const state = {
  gain: 0, output: 0, threshold: 0, knee: 50,
  style: 0, mode: 0, oversampling: 0, signal: 1, clipper: true,
  inputDb: -60, outputDb: -60, grDb: 0,
}

const styleNames = ['SOFT', 'MEDIUM', 'HARD']
const modeNames = ['STEREO', 'M/S', 'MULTIBAND']
const osNames = ['1x', '2x', '4x', '8x']
const signalNames = ['IN', 'GR', 'OUT']
const signalTitles = ['INPUT', 'GAIN REDUCTION', 'OUTPUT']
const embedded = () => !!window.__JUCE__?.backend
function emitNative(eventId, payload = {}) { if (embedded()) window.__JUCE__.backend.emitEvent(eventId, payload) }
const app = document.querySelector('#app')

app.innerHTML = `
<main class="chassis">
<header class="header"><div class="brand-left"><h1>SOFT CLIP</h1><span>CLEAN LOUDER TOGETHER</span></div><div class="brand-right"><strong>WADIDAW</strong><small>AUDIO TOOLS</small></div></header>
<section class="panel meters-panel"><span class="screw s1"></span><span class="screw s2"></span><span class="screw s3"></span><span class="screw s4"></span>
<div class="meter-pair meter-in"><div class="meter-stack"><div class="meter"><i id="inL"></i></div><div class="meter"><i id="inR"></i></div></div><output id="inputReadout">-60.0 dB<small>LEVEL IN</small></output></div>
<div class="meter-pair meter-out"><div class="meter-stack"><div class="meter"><i id="outL"></i></div><div class="meter"><i id="outR"></i></div></div><output id="outputReadout">-60.0 dB<small>LEVEL OUT</small></output></div></section>
<section class="panel center-panel"><span class="screw s1"></span><span class="screw s2"></span><span class="screw s3"></span><span class="screw s4"></span>
<div class="gr-display"><div class="grid"></div><div class="gr-title" id="signalTitle">GAIN REDUCTION</div><div class="gr-value" id="grValue">0.0 dB</div><div class="needle" id="needle"></div><div class="needle-value" id="needleValue">0.0 dB</div></div>
<div class="top-knobs"><div class="control"><label>THRESHOLD</label><div class="knob" id="thresholdKnob"><span></span></div><output id="thresholdValue">0.0 dB</output></div><div class="control"><label>KNEE</label><div class="knob" id="kneeKnob"><span></span></div><output id="kneeValue">50 %</output></div></div>
<div class="bottom-controls"><div class="control large"><label>GAIN</label><div class="knob" id="gainKnob"><span></span></div><output id="gainValue">+0.0 dB</output></div><div class="signal-control"><label>SIGNAL</label><div class="signal-switch toggle-multi" id="signalSwitch"><span></span></div><output id="signalValue">GR</output></div><div class="control large"><label>OUTPUT</label><div class="knob" id="outputKnob"><span></span></div><output id="outputValue">+0.0 dB</output></div></div></section>
<section class="panel right-panel"><span class="screw s1"></span><span class="screw s2"></span><span class="screw s3"></span><span class="screw s4"></span>
<div class="choice-control"><label>CLIPPER STYLE</label><div class="toggle-control toggle-multi" data-key="style"><span></span></div><output id="styleValue">SOFT</output></div>
<div class="choice-control"><label>MODE</label><div class="toggle-control toggle-multi" data-key="mode"><span></span></div><output id="modeValue">STEREO</output></div>
<div class="choice-control"><label>OVERSAMPLING</label><div class="toggle-control toggle-multi" data-key="oversampling"><span></span></div><output id="osValue">1x</output></div></section>
<section class="panel clipper-panel"><span class="screw s1"></span><span class="screw s2"></span><span class="screw s3"></span><span class="screw s4"></span><button id="clipperToggle" class="toggle on"><span></span><b>CLIPPER</b><em>ON</em></button></section>
<footer><span>WADIDAW AUDIO TOOLS</span><strong>CLEAN LOUDER TOGETHER</strong><span>EST. 2025</span></footer></main>`

const $ = id => document.getElementById(id)
const keyFor = id => id === 'gainKnob' ? 'gain' : id === 'outputKnob' ? 'output' : id === 'thresholdKnob' ? 'threshold' : 'knee'

function setKnob(el, value, min, max, output, formatter) {
  const p = Math.max(0, Math.min(1, (value - min) / (max - min)))
  el.style.setProperty('--angle', `${-135 + p * 270}deg`)
  output.textContent = formatter(value)
}

function updateMultiToggle(selector, value, max) {
  const el = typeof selector === 'string' ? document.querySelector(selector) : selector
  if (!el) return
  el.querySelector('span').style.left = `${(value / max) * 100}%`
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
  $('signalTitle').textContent = signalTitles[state.signal] ?? 'GAIN REDUCTION'
  $('clipperToggle').classList.toggle('on', state.clipper)
  $('clipperToggle em').textContent = state.clipper ? 'ON' : 'OFF'

  const ip = Math.max(0, Math.min(100, ((state.inputDb + 60) / 60) * 100))
  const op = Math.max(0, Math.min(100, ((state.outputDb + 60) / 60) * 100))
  ;['inL', 'inR'].forEach(id => $(id).style.height = `${ip}%`)
  ;['outL', 'outR'].forEach(id => $(id).style.height = `${op}%`)
  $('inputReadout').firstChild.textContent = `${state.inputDb.toFixed(1)} dB`
  $('outputReadout').firstChild.textContent = `${state.outputDb.toFixed(1)} dB`

  const value = state.grDb <= -0.01 ? state.grDb : 0
  $('grValue').textContent = `${value.toFixed(1)} dB`
  $('needleValue').textContent = `${value.toFixed(1)} dB`
  const amount = Math.max(0, Math.min(20, Math.abs(value)))
  const angle = -55 + amount * 5
  $('needle').style.transform = `rotate(${angle}deg)`
  $('needleValue').style.setProperty('--needle-angle', `${angle * -1}deg`)

  updateMultiToggle('[data-key="style"]', state.style, 2)
  updateMultiToggle('[data-key="mode"]', state.mode, 2)
  updateMultiToggle('[data-key="oversampling"]', state.oversampling, 3)
  updateMultiToggle('#signalSwitch', state.signal, 2)
}

function setParameter(id, value) { emitNative('setParameter', { id, value }) }
function cycle(key, max, parameterId) { state[key] = (state[key] + 1) % max; setParameter(parameterId, state[key]); render() }

function bindKnob(id, min, max, parameterId) {
  const el = $(id), key = keyFor(id)
  let startY = 0, startValue = 0, dragging = false
  el.addEventListener('pointerdown', e => { dragging = true; startY = e.clientY; startValue = state[key]; el.setPointerCapture(e.pointerId) })
  el.addEventListener('pointermove', e => { if (!dragging) return; const value = Math.max(min, Math.min(max, startValue + (startY - e.clientY) * (max - min) / 180)); state[key] = key === 'knee' ? Math.round(value) : Math.round(value * 10) / 10; setParameter(parameterId, state[key]); render() })
  el.addEventListener('pointerup', () => dragging = false)
  el.addEventListener('pointercancel', () => dragging = false)
}

bindKnob('gainKnob', -12, 12, 'input_gain')
bindKnob('outputKnob', -12, 12, 'output_gain')
bindKnob('thresholdKnob', -20, 0, 'threshold')
bindKnob('kneeKnob', 0, 100, 'knee')
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
