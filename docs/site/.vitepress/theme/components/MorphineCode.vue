<script setup lang="ts">
import {computed, nextTick, ref, watch} from "vue";
import {codeToHtml} from 'shiki'
import {bundledThemesInfo} from 'shiki/themes'

function getInput() {
  let inputRef = ref("")
  const text = localStorage.getItem("morphine-playground-code")
  if (text == null) {
    fetch("morphine/playground.ms")
        .then(async (response) => inputRef.value = await response.text())
  } else {
    inputRef.value = text
  }

  return inputRef
}

const model = defineModel()

let themeRef = ref('vitesse-dark')
let highlightRef = ref()
let codeareaRef = ref()
let inputRef = getInput()
let outputRef = ref()
let preStyleRef = ref()

const currentThemeType = computed(() => bundledThemesInfo.find(i => i.id === themeRef.value)?.type || 'inherit')

watch(inputRef, hlrun, {immediate: true})

async function hlrun() {
  model.value = inputRef.value
  localStorage.setItem("morphine-playground-code", inputRef.value)

  outputRef.value = await codeToHtml(inputRef.value, {
    lang: 'js',
    theme: themeRef.value,
    transformers: [
      {
        preprocess(code) {
          if (code.endsWith('\n')) {
            return `${code}\n`
          }
        },
        pre(node) {
          this.addClassToHast(node, 'vp-code')
          this.addClassToHast(node, 'overflow-auto')
          this.addClassToHast(node, 'w-full')
          this.addClassToHast(node, 'h-full')
          this.addClassToHast(node, 'px-8')
          this.addClassToHast(node, 'py-6')
          this.addClassToHast(node, 'm-0')
          preStyleRef.value = node.properties?.style as string || ''
        },
      }
    ]
  })

  nextTick().then(() => {
    syncScroll()
  })
}

function syncScroll() {
  if (!highlightRef.value || !codeareaRef.value) {
    return
  }

  const preEl = highlightRef.value.children[0] as HTMLPreElement
  if (!preEl) {
    return
  }

  preEl.scrollTop = codeareaRef.value.scrollTop
  preEl.scrollLeft = codeareaRef.value.scrollLeft
}

function onInput() {
  nextTick().then(() => {
    syncScroll()
  })
}

function handleTab(e) {
  e.preventDefault()

  let area = codeareaRef.value
  let start = area.selectionStart;
  let end = area.selectionEnd;

  area.value = area.value.substring(0, start) + "\t" + area.value.substring(end);
  area.selectionStart = area.selectionEnd = start + 1;

  inputRef.value = area.value
  onInput()
}
</script>

<template>
  <div class="vp-adaptive-theme rounded-lg overflow-hidden w-full h-full"
       :style="[preStyleRef, { colorScheme: currentThemeType }]">
    <div class="relative h-96">
      <span ref="highlightRef" v-html="outputRef"/>
      <textarea
          ref="codeareaRef"
          class="codearea m-0 px-8 py-6 bg-clip-padding w-full h-full absolute inset-0 text-transparent bg-transparent whitespace-pre overflow-auto resize-none caret-gray-400 z-10"
          autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false"
          v-model="inputRef"
          @input="onInput"
          @scroll="syncScroll"
          @keydown.tab="handleTab"/>
    </div>
  </div>
</template>

<style scoped>
.codearea {
  line-height: inherit;
  font-family: var(--vp-font-family-mono);
  font-feature-settings: normal;
  font-variation-settings: normal;
  font-size: 1em;
  tab-size: 4;
}
</style>