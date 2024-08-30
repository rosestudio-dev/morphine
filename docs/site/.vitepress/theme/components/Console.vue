<script setup lang="ts">
import {ref, watch} from "vue";

const model = defineModel()
const props = defineProps<{ name?: String }>()

let areaRef = ref()

watch(model, autoScroll, {immediate: true})

function autoScroll() {
  if (!areaRef.value) {
    return
  }

  areaRef.value.scrollTop = areaRef.value.scrollHeight;
}
</script>

<template>
  <div class="vp-adaptive-theme rounded-lg overflow-hidden w-full h-full">
    <div class="relative h-64">
      <span
          class="opacity-15 hover:opacity-85 transition duration-300 ease-out absolute top-2 right-2 text-neutral-200 bg-neutral-900 rounded-lg px-2 py-1 select-none z-10">
        {{ props.name }}
      </span>
      <span
          ref="areaRef"
          class="area m-0 px-8 py-6 bg-clip-padding w-full h-full absolute inset-0 bg-transparent whitespace-pre overflow-auto resize-none caret-gray-400">
        {{ model }}
      </span>
    </div>
  </div>
</template>

<style scoped>
.area {
  line-height: inherit;
  font-family: var(--vp-font-family-mono);
  font-feature-settings: normal;
  font-variation-settings: normal;
  font-size: 1em;
  tab-size: 4;
}
</style>