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
<div class="gr-display"><div class="grid"></div><div class="gr-title" id="signalTitle">GAIN REDUCTION</div><div class="gr-value" id="grValue">0.0 dB</div><div class="vu-meters" id="vuMeters"></div></div>
<div class="top-knobs"><div class="control"><label>THRESHOLD</label><div class="knob" id="thresholdKnob"><span></span></div><output id="thresholdValue">0.0 dB</output></div><div class="control"><label>KNEE</label><div class="knob" id="kneeKnob"><span></span></div><output id="kneeValue">50 %</output></div></div>
<div class="bottom-controls"><div class="control large"><label>GAIN</label><div class="knob" id="gainKnob"><span></span></div><output id="gainValue">+0.0 dB</output></div><div class="signal-control"><label>SIGNAL</label><div class="signal-switch toggle-multi" id="signalSwitch"><span></span></div><output id="signalValue">GR</output></div><div class="control large"><label>OUTPUT</label><div class="knob" id="outputKnob"><span></span></div><output id="outputValue">+0.0 dB</output></div></div></section>
<section class="panel right-panel"><span class="screw s1"></span><span class="screw s2"></span><span class="screw s3"></span><span class="screw s4"></span>
<div class="choice-control"><label>CLIPPER STYLE</label><div class="toggle-control toggle-multi" data-key="style"><span></span></div><output id="styleValue">SOFT</output></div>
<div class="choice-control"><label>MODE</label><div class="toggle-control toggle-multi" data-key="mode"><span></span></div><output id="modeValue">STEREO</output></div>
<div class="choice-control"><label>OVERSAMPLING</label><div class="toggle-control toggle-multi" data-key="oversampling"><span></span></div><output id="osValue">1x</output></div></section>
<section class="panel clipper-panel"><span class="screw s1"></span><span class="screw s2"></span><span class="screw s3"></span><span class="s4"></span><button id="clipperToggle" class="toggle on"><span></span><b>CLIPPER</b><em>ON</em></button></section>
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
  const p = max > 0 ? Math.max(0, Math.min(1, value / max)) : 0
  el.style.setProperty('--toggle-position', p)
  const thumb = el.querySelector('span')
  if (thumb) thumb.style.left = `calc(10px + ${p * 100}% - ${p * 20}px)`
}

function updateSignalToggle(value) {
  const el = $('signalSwitch')
  if (!el) return
  const p = Math.max(0, Math.min(1, value / 2))
  const thumb = el.querySelector('span')
  if (thumb) {
    thumb.style.left = '50%'
    thumb.style.top = `calc(4px + ${p * 100}% - ${p * 18}px)`
  }
}

const meterScale = ['+3', '0', '-5', '-10', '-20']
const meterCountForSignal = () => state.signal === 1 ? (state.mode === 2 ? 3 : state.mode === 1 ? 2 : 1) : 1
let meterSignature = ''

function buildVUMeters() {
  const container = $('vuMeters')
  const count = meterCountForSignal()
  const labels = state.signal === 1
    ? (state.mode === 2 ? ['LOW', 'MID', 'HIGH'] : state.mode === 1 ? ['MID', 'SIDE'] : ['GR'])
    : [state.signal === 0 ? 'INPUT' : 'OUTPUT']
  const signature = `${state.signal}:${state.mode}:${count}:${labels.join('|')}`
  if (signature === meterSignature) return
  meterSignature = signature
  container.innerHTML = Array.from({ length: count }, (_, index) => `
    <div class="vu-meter" data-index="${index}">
      <div class="vu-scale">${meterScale.map(v => `<span>${v}</span>`).join('')}</div>
      <div class="vu-face">
        <div class="vu-ticks"></div>
        <div class="vu-needle" id="vuNeedle${index}"></div>
        <div class="vu-center"></div>
      </div>
      <div class="vu-label">${labels[index]}</div>
      <output id="vuReadout${index}">0.0 dB</output>
    </div>`).join('')
}

function valueForMeter(index) {
  if (state.signal === 0) return state.inputDb
  if (state.signal === 2) return state.outputDb
  const base = Math.abs(Math.min(0, state.grDb))
  if (state.mode === 1) return index === 0 ? -base : -base * 0.82
  if (state.mode === 2) return index === 0 ? -base * 0.7 : index === 1 ? -base : -base * 0.86
  return -base
}

function renderVUMeters() {
  const count = meterCountForSignal()
  for (let i = 0; i < count; i++) {
    const value = valueForMeter(i)
    const db = Math.max(-20, Math.min(3, value))
    const p = (db + 20) / 23
    const angle = -55 + p * 110
    const needle = $(`vuNeedle${i}`)
    const readout = $(`vuReadout${i}`)
    if (needle) needle.style.transform = `rotate(${angle}deg)`
    if (readout) readout.textContent = `${db.toFixed(1)} dB`
  }
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

  const displayValue = state.signal === 0 ? state.inputDb : state.signal === 2 ? state.outputDb : state.grDb
  $('grValue').textContent = `${displayValue.toFixed(1)} dB`
  buildVUMeters()
  renderVUMeters()

  updateMultiToggle('[data-key="style"]', state.style, 2)
  updateMultiToggle('[data-key="mode"]', state.mode, 2)
  updateMultiToggle('[data-key="oversampling"]', state.oversampling, 3)
  updateSignalToggle(state.signal)
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
