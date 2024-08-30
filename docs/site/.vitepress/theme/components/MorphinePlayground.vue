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

function inputClick() {
  if (runningRef.value) {
    return
  }

  const chars = inputRef.value + "\n"
  for(const c of chars) {
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
        outputConsoleRef.value += msg.data.text
        break
      }
      case "stderr": {
        errorConsoleRef.value += msg.data.text
        break
      }
    }
  }

  worker.postMessage({code: codeInputRef.value, stdin})
  stdin = []
})
</script>

<template>
  <div class="flex flex-col gap-8 mt-4 bg-transparent rounded-xl" :class="{dark: isDark}">
    <MorphineCode class="shadow-lg shadow-zinc-700 dark:shadow-zinc-900" v-model="codeInputRef"/>
    <div class="flex flex-row justify-end items-stretch gap-4 h-12">
      <div :class="{'opacity-60': runningRef, 'hover:scale-[1.02]': !runningRef}"
          class="shadow-lg shadow-zinc-400 dark:shadow-zinc-900 bg-neutral-300 dark:bg-neutral-800 w-full text-md rounded-lg flex flex-row justify-end overflow-hidden transition duration-300 ease-out">
        <input :disabled="runningRef" v-model="inputRef" type="text"
               class="ms-4 me-2 my-2 w-full text-neutral-900 placeholder-neutral-500 dark:text-neutral-300"
               placeholder="input"/>
        <button :disabled="runningRef" @click="inputClick" class="bg-neutral-800 dark:bg-neutral-500 w-12 flex justify-center items-center">
          <svg class="fill-neutral-300 dark:fill-neutral-800 w-7 h-7" xmlns="http://www.w3.org/2000/svg" height="24px"
               viewBox="0 -960 960 960" width="24px" fill="#e8eaed">
            <path
                d="M176-183q-20 8-38-3.5T120-220v-180l320-80-320-80v-180q0-22 18-33.5t38-3.5l616 260q25 11 25 37t-25 37L176-183Z"/>
          </svg>
        </button>
      </div>
      <div
          class="shadow-lg shadow-zinc-700 dark:shadow-zinc-900 bg-neutral-800 rounded-lg w-12 h-12 transition duration-300 ease-out hover:scale-105">
        <button @click="runClick" class="w-full h-full relative">
          <transition>
            <svg v-if="runningRef" class="inset-0 m-auto absolute fill-rose-400 w-7 h-7"
                 xmlns="http://www.w3.org/2000/svg" height="24px"
                 viewBox="0 -960 960 960" width="24px" fill="#e8eaed">
              <path
                  d="M360-320h240q17 0 28.5-11.5T640-360v-240q0-17-11.5-28.5T600-640H360q-17 0-28.5 11.5T320-600v240q0 17 11.5 28.5T360-320ZM480-80q-83 0-156-31.5T197-197q-54-54-85.5-127T80-480q0-83 31.5-156T197-763q54-54 127-85.5T480-880q83 0 156 31.5T763-763q54 54 85.5 127T880-480q0 83-31.5 156T763-197q-54 54-127 85.5T480-80Z"/>
            </svg>
            <svg v-else class="inset-0 m-auto absolute fill-emerald-600 w-7 h-7" xmlns="http://www.w3.org/2000/svg"
                 height="24px"
                 viewBox="0 -960 960 960" width="24px" fill="#e8eaed">
              <path
                  d="m426-330 195-125q14-9 14-25t-14-25L426-630q-15-10-30.5-1.5T380-605v250q0 18 15.5 26.5T426-330Zm54 250q-83 0-156-31.5T197-197q-54-54-85.5-127T80-480q0-83 31.5-156T197-763q54-54 127-85.5T480-880q83 0 156 31.5T763-763q54 54 85.5 127T880-480q0 83-31.5 156T763-197q-54 54-127 85.5T480-80Z"/>
            </svg>
          </transition>
        </button>
      </div>
    </div>
    <div class="flex flex-col md:flex-row justify-stretch gap-6">
      <Console
          class="shadow-lg shadow-zinc-400 dark:shadow-zinc-900 bg-neutral-300 dark:bg-neutral-800 text-neutral-900 dark:text-neutral-200"
          v-model="outputConsoleRef" name="output console"/>
      <Console
          class="shadow-lg shadow-zinc-400 dark:shadow-zinc-900 bg-neutral-300 dark:bg-neutral-800 text-rose-800 dark:text-rose-300"
          v-model="errorConsoleRef" name="error console"/>
    </div>
  </div>
</template>

<style scoped>
.v-enter-active,
.v-leave-active {
  transition: opacity 0.3s ease-out;
}

.v-enter-from,
.v-leave-to {
  opacity: 0;
}
</style>