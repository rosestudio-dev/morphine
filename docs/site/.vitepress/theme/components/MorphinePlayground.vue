<script setup lang="ts">
import {ref} from "vue";
import MorphineCode from "./MorphineCode.vue";
import Console from "./Console.vue";
import {debounce} from 'throttle-debounce';
import {useData} from "vitepress";

const {isDark} = useData()

let outputConsoleRef = ref("")
let errorConsoleRef = ref("")

let runningRef = ref(false)

let codeInputRef = ref("")
let inputRef = ref("")

let worker = undefined
let stdin = []

const errorConsoleLang = {
  "displayName": "errorConsole",
  "name": "errorConsole",
  "patterns": [
    {
      "match": ".*",
      "name": "markup.deleted.errorConsole"
    }
  ],
  "scopeName": "source.errorConsole"
}

function terminate(print) {
  setTimeout(() => {
    if (typeof (worker) != "undefined") {
      worker.terminate()
      worker = undefined
      runningRef.value = false

      if (typeof (print) != "undefined") {
        errorConsoleRef.value += print + "\n"
      }
    }
  }, 0)
}

function updateText(ref, msg) {
  let text = ref.value + msg
  if (text.length > 16384) {
    text = text.substring(text.length - 16384, text.length)
  }
  ref.value = text
}

function inputClick() {
  if (runningRef.value) {
    return
  }

  const chars = inputRef.value + "\n"
  for (const c of chars) {
    stdin.push(c.charCodeAt(0))
  }
  inputRef.value = ""
}

const runClick = debounce(200, () => {
  if (runningRef.value) {
    terminate("[Terminate]")
    return
  }

  runningRef.value = true
  outputConsoleRef.value = ""
  errorConsoleRef.value = ""

  worker = new Worker(new URL('../workers/morphine.js', import.meta.url), {type: "module"})
  worker.onerror = function (err) {
    terminate("[Worker (unrec): " + err.message + "]")
  }
  worker.onmessage = function (msg) {
    switch (msg.data.type) {
      case "success": {
        terminate()
        break
      }
      case "error": {
        terminate("[Worker (rec): " + msg.data.error + "]")
        break
      }
      case "stdout": {
        updateText(outputConsoleRef, msg.data.text)
        break
      }
      case "stderr": {
        updateText(errorConsoleRef, msg.data.text)
        break
      }
    }
  }

  worker.postMessage({code: codeInputRef.value, stdin})
  stdin = []
})
</script>

<template>
  <div class="h-[150vh] lg:h-[80vh] grid grid-cols-1 grid-rows-5 lg:grid-cols-5 lg:grid-rows-1 mt-4 gap-4 p-4 md:gap-8 md:p-8 rounded-xl bg-[var(--vp-c-default-soft)]"
       :class="{dark: isDark}">
    <div class="w-full h-full row-span-2 lg:row-span-1 lg:col-span-3 flex flex-col items-stretch gap-4 md:gap-8">
      <MorphineCode class="w-full h-full shadow-lg shadow-zinc-400 dark:shadow-zinc-900" v-model="codeInputRef"/>
      <div class="flex flex-row justify-end items-stretch gap-2 md:gap-4 h-12">
        <div :class="{'opacity-60': runningRef, 'hover:scale-[1.02]': !runningRef}"
             class="shadow-lg shadow-zinc-400 dark:shadow-zinc-900 w-full text-md rounded-lg flex flex-row justify-end overflow-hidden transition duration-300 ease-out">
          <input :disabled="runningRef" v-model="inputRef" type="text"
                 class="ps-4 pe-2 py-2 w-full bg-[var(--mpg-c-bg)] text-[var(--vp-c-neutral-inverse)] placeholder-[var(--vp-c-gray-3)]"
                 placeholder="input"/>
          <button :disabled="runningRef" @click="inputClick"
                  class="bg-[var(--mpg-c-fg)] w-12 flex justify-center items-center">
            <svg class="fill-[var(--mpg-c-bg)] w-7 h-7" xmlns="http://www.w3.org/2000/svg" height="24px"
                 viewBox="0 -960 960 960" width="24px" fill="#e8eaed">
              <path
                  d="M176-183q-20 8-38-3.5T120-220v-180l320-80-320-80v-180q0-22 18-33.5t38-3.5l616 260q25 11 25 37t-25 37L176-183Z"/>
            </svg>
          </button>
        </div>
        <div
            class="shadow-lg shadow-zinc-700 dark:shadow-zinc-900 bg-[--mpg-c-bg] dark:bg-[--mpg-c-fg] rounded-lg w-12 h-12 transition duration-300 ease-out hover:scale-105">
          <button @click="runClick" class="w-full h-full relative">
            <transition>
              <svg v-if="runningRef" class="inset-0 m-auto absolute fill-[--vp-c-red-2] w-7 h-7"
                   xmlns="http://www.w3.org/2000/svg" height="24px"
                   viewBox="0 -960 960 960" width="24px" fill="#e8eaed">
                <path
                    d="M360-320h240q17 0 28.5-11.5T640-360v-240q0-17-11.5-28.5T600-640H360q-17 0-28.5 11.5T320-600v240q0 17 11.5 28.5T360-320ZM480-80q-83 0-156-31.5T197-197q-54-54-85.5-127T80-480q0-83 31.5-156T197-763q54-54 127-85.5T480-880q83 0 156 31.5T763-763q54 54 85.5 127T880-480q0 83-31.5 156T763-197q-54 54-127 85.5T480-80Z"/>
              </svg>
              <svg v-else class="inset-0 m-auto absolute fill-[--vp-c-green-3] w-7 h-7" xmlns="http://www.w3.org/2000/svg"
                   height="24px"
                   viewBox="0 -960 960 960" width="24px" fill="#e8eaed">
                <path
                    d="m426-330 195-125q14-9 14-25t-14-25L426-630q-15-10-30.5-1.5T380-605v250q0 18 15.5 26.5T426-330Zm54 250q-83 0-156-31.5T197-197q-54-54-85.5-127T80-480q0-83 31.5-156T197-763q54-54 127-85.5T480-880q83 0 156 31.5T763-763q54 54 85.5 127T880-480q0 83-31.5 156T763-197q-54 54-127 85.5T480-80Z"/>
              </svg>
            </transition>
          </button>
        </div>
      </div>
    </div>
    <div class="w-full h-full row-span-3 lg:row-span-1 lg:col-span-2 flex flex-col-reverse lg:flex-col items-stretch gap-4 md:gap-8">
      <Console
          class="w-full h-full shadow-lg shadow-zinc-400 dark:shadow-zinc-900"
          v-model="errorConsoleRef" name="error console" :lang="errorConsoleLang" />
      <Console
          class="w-full h-full shadow-lg shadow-zinc-400 dark:shadow-zinc-900"
          v-model="outputConsoleRef" name="output console" lang="ansi"/>
    </div>
  </div>
</template>

<style>
:root {
  --mpg-c-fg: #e2e2e3;
  --mpg-c-bg: #161618;
}

.dark {
  --mpg-c-fg: #161618;
  --mpg-c-bg: #e2e2e3;
}

.v-enter-active,
.v-leave-active {
  transition: opacity 0.3s ease-out;
}

.v-enter-from,
.v-leave-to {
  opacity: 0;
}
</style>