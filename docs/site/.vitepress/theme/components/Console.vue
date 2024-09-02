<script setup lang="ts">
import {computed, nextTick, ref, watch} from "vue";
import {codeToHtml} from "shiki";
import {useData} from "vitepress";
import {bundledThemesInfo} from "shiki/themes";
import {throttle} from 'throttle-debounce';

const {isDark} = useData()
const model = defineModel()
const props = defineProps(["name", "lang"])

const currentThemeType = computed(() => bundledThemesInfo.find(i => i.id === themeRef.value)?.type || 'inherit')
let themeRef = computed(() => isDark.value ? 'vitesse-dark' : 'vitesse-light')
let areaRef = ref()
let outputRef = ref()
let preStyleRef = ref()
let hasScrollBarRef = ref(false)

watch(model, throttle(250, async () => { await hlrun() }), {immediate: true})
watch(themeRef, hlrun, {immediate: true})

watch(outputRef, autoScroll, {immediate: true})

function hasScrollBar(elem) {
  if (!elem) {
    return false
  }

  return elem.scrollHeight > elem.clientHeight
}

async function hlrun() {
  outputRef.value = await codeToHtml(model.value, {
    lang: props.lang,
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
          this.addClassToHast(node, 'consolelinepart')
          this.addClassToHast(node, 'consoletext')
        },
        line(node) {
          node.tagName = "pre"
          if (node.children.length === 0) {
            node.children.push({
              type: "element",
              tagName: "div",
              properties: {
                class: ['consolelinepart', 'consoletext']
              },
              children: [
                {type: "text", value: " "}
              ]
            })
          }
          this.addClassToHast(node, 'consoleline')
        },
        code(node) {
          node.tagName = "div"
        }
      }
    ]
  })
}

function autoScroll() {
  if (!areaRef.value) {
    return
  }

  nextTick(() => {
    areaRef.value.scrollTop = areaRef.value.scrollHeight;
    hasScrollBarRef.value = hasScrollBar(areaRef.value)
  })
}
</script>

<template>
  <div class="vp-adaptive-theme rounded-lg overflow-hidden w-full h-full"
       :style="[preStyleRef, { colorScheme: currentThemeType }]">
    <div class="relative w-full h-full">
      <span
          class="opacity-15 hover:opacity-85 transition duration-300 ease-out absolute top-4 text-neutral-200 bg-neutral-800 rounded-lg px-2 py-1 select-none z-10"
          :class="{'right-4':!hasScrollBarRef, 'right-6':hasScrollBarRef}">
        {{ props.name }}
      </span>
      <div
          ref="areaRef"
          class="consoletext overflow-auto w-full h-full"
          v-html="outputRef"/>
    </div>
  </div>
</template>

<style>
.consoletext {
  @apply text-xs;

  font-family: var(--vp-font-family-mono);
  font-feature-settings: normal;
  font-variation-settings: normal;
  tab-size: 4;
}

.consoleline {
  @apply block m-0 p-0;
  line-height: 0;
}

.consolelinepart {
  @apply inline-block;
}
</style>