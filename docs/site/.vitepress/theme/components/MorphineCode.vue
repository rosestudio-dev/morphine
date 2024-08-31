<script setup lang="ts">
import {computed, nextTick, ref, watch} from "vue";
import {codeToHtml} from 'shiki'
import {bundledThemesInfo} from 'shiki/themes'
import {useData} from "vitepress";

const model = defineModel()
const {isDark} = useData()

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

function getLang() {
  let langRef = ref("text")
  fetch("json/morphine-grammar.json")
      .then(async (response) => langRef.value = JSON.parse(await response.text()))

  return langRef
}

const currentThemeType = computed(() => bundledThemesInfo.find(i => i.id === themeRef.value)?.type || 'inherit')
let themeRef = computed(() => isDark.value ? 'vitesse-dark' : 'vitesse-light')
let langRef = getLang()
let highlightRef = ref()
let codeareaRef = ref()
let inputRef = getInput()
let outputRef = ref()
let preStyleRef = ref()

watch(inputRef, hlrun, {immediate: true})
watch(themeRef, hlrun, {immediate: true})
watch(langRef, hlrun, {immediate: true})

async function hlrun() {
  model.value = inputRef.value
  localStorage.setItem("morphine-playground-code", inputRef.value)

  outputRef.value = await codeToHtml(inputRef.value, {
    lang: langRef.value,
    theme: themeRef.value,
    transformers: [
      {
        preprocess(code) {
          const formated = code.replaceAll("\n\n", "\n \n")
          if (formated.endsWith('\n')) {
            return `${formated}\n`
          } else {
            return formated
          }
        },
        pre(node) {
          node.tagName = "div"
          this.addClassToHast(node, 'vp-code')
          this.addClassToHast(node, 'w-fit')
          this.addClassToHast(node, 'h-fit')
          this.addClassToHast(node, 'px-8')
          this.addClassToHast(node, 'py-6')
          this.addClassToHast(node, 'm-0')
          preStyleRef.value = node.properties?.style as string || ''
        },
        span(node) {
          node.tagName = "div"
          this.addClassToHast(node, 'codepart')
          this.addClassToHast(node, 'codetext')
        },
        line(node) {
          node.tagName = "pre"
          this.addClassToHast(node, 'codeline')
        },
        code(node) {
          node.tagName = "div"
        }
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

  highlightRef.value.scrollTop = codeareaRef.value.scrollTop
  highlightRef.value.scrollLeft = codeareaRef.value.scrollLeft
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
    <div class="relative w-full h-full">
      <div ref="highlightRef" class="select-none overflow-auto w-full h-full" v-html="outputRef"/>
      <textarea
          ref="codeareaRef"
          class="codetext absolute inset-0 z-10 overflow-auto box-content px-8 py-6 text-transparent bg-transparent whitespace-pre resize-none caret-gray-400"
          autocomplete="off" autocapitalize="off" spellcheck="false"
          contenteditable="true"
          v-model="inputRef"
          @input="onInput"
          @scroll="syncScroll"
          @keydown.tab="handleTab"/>
    </div>
  </div>
</template>

<style>
.codetext {
  @apply text-sm;

  font-family: var(--vp-font-family-mono);
  font-feature-settings: normal;
  font-variation-settings: normal;
  tab-size: 4;
}

.codeline {
  @apply block m-0 p-0;
  line-height: 0;
}

.codepart {
  @apply inline-block;
}
</style>