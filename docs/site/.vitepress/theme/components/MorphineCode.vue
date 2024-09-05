<script setup lang="ts">
import {computed, nextTick, ref, watch} from "vue";
import {codeToHtml} from 'shiki'
import {bundledThemesInfo} from 'shiki/themes'
import {useData} from "vitepress";
import grammarRaw from '../grammar/morphine.json?raw';

import script1Raw from '../playground/script1.ms?raw'
import script2Raw from '../playground/script2.ms?raw'
import script3Raw from '../playground/script3.ms?raw'
import script4Raw from '../playground/script4.ms?raw'
import script5Raw from '../playground/script5.ms?raw'
import script6Raw from '../playground/script6.ms?raw'
import script7Raw from '../playground/script7.ms?raw'

const scripts = [
  script1Raw,
  script2Raw,
  script3Raw,
  script4Raw,
  script5Raw,
  script6Raw,
  script7Raw
]

const model = defineModel()
const {isDark} = useData()

function getInput() {
  let inputRef = ref("")
  const text = localStorage.getItem("morphine-playground-code")
  if (text == null) {
    inputRef.value = scripts[Math.floor(Math.random() * scripts.length)]
  } else {
    inputRef.value = text
  }

  return inputRef
}

function getLang() {
  return ref(JSON.parse(grammarRaw))
}

const currentThemeType = computed(() => bundledThemesInfo.find(i => i.id === themeRef.value)?.type || 'inherit')
let themeRef = computed(() => isDark.value ? 'vitesse-dark' : 'vitesse-light')
let langRef = getLang()
let highlightRef = ref()
let codeareaRef = ref()
let inputRef = getInput()
let outputRef = ref()
let preStyleRef = ref()
let hasScrollBarRef = ref(false)

watch(inputRef, hlrun, {immediate: true})
watch(themeRef, hlrun, {immediate: true})
watch(langRef, hlrun, {immediate: true})

function hasScrollBar(elem) {
  if (!elem) {
    return false
  }

  return elem.scrollHeight > elem.clientHeight
}

function changeScript() {
  let index = Math.floor(Math.random() * scripts.length)
  let result = scripts[index];
  if (result == inputRef.value) {
    result = (index + 1) % scripts.length
  }

  inputRef.value = result
}

async function hlrun() {
  model.value = inputRef.value
  localStorage.setItem("morphine-playground-code", inputRef.value)

  outputRef.value = await codeToHtml(inputRef.value, {
    lang: langRef.value,
    theme: themeRef.value,
    transformers: [
      {
        preprocess(code) {
          if (code.endsWith('\n')) {
            return `${code}\n`
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
          this.addClassToHast(node, 'codelinepart')
          this.addClassToHast(node, 'codetext')
        },
        line(node) {
          node.tagName = "pre"
          if (node.children.length === 0) {
            node.children.push({
              type: "element",
              tagName: "div",
              properties: {
                class: ['codelinepart', 'codetext']
              },
              children: [
                {type: "text", value: " "}
              ]
            })
          }
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
  hasScrollBarRef.value = hasScrollBar(highlightRef.value)
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
      <div
          class="opacity-15 hover:opacity-85 transition duration-300 ease-out absolute top-4 text-neutral-200 bg-neutral-800 rounded-lg px-2 py-1 select-none z-10"
          :class="{'right-4':!hasScrollBarRef, 'right-6':hasScrollBarRef}"
          @click="changeScript">
        <svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#e8eaed">
          <path
              d="M600-160q-17 0-28.5-11.5T560-200q0-17 11.5-28.5T600-240h64l-99-99q-12-12-11.5-28.5T566-396q12-12 28.5-12t28.5 12l97 98v-62q0-17 11.5-28.5T760-400q17 0 28.5 11.5T800-360v160q0 17-11.5 28.5T760-160H600Zm-428-12q-11-11-11-28t11-28l492-492h-64q-17 0-28.5-11.5T560-760q0-17 11.5-28.5T600-800h160q17 0 28.5 11.5T800-760v160q0 17-11.5 28.5T760-560q-17 0-28.5-11.5T720-600v-64L228-172q-11 11-28 11t-28-11Zm-1-560q-11-11-11-28t11-28q11-11 27.5-11t28.5 11l168 167q11 11 11.5 27.5T395-565q-11 11-28 11t-28-11L171-732Z"/>
        </svg>
      </div>
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

.codelinepart {
  @apply inline-block;
}
</style>