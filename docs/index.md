---
layout: home

hero:
  name: "Morphine"
  text: "Programming Language"
  actions:
    - theme: brand
      text: Why Morphine?
    - theme: alt
      text: Quickstart
      link: /quickstart
    - theme: alt
      text: GitHub
      link: https://github.com/rosestudio-dev/morphine-lang
  image:
    light: /assets/logo-small-light.svg
    dark: /assets/logo-small-dark.svg
features:
  - icon: <svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 -960 960 960" width="24"> <path id="feature-icon-path" d="M360-400v-160q0-17 11.5-28.5T400-600h160q17 0 28.5 11.5T600-560v160q0 17-11.5 28.5T560-360H400q-17 0-28.5-11.5T360-400Zm80-40h80v-80h-80v80Zm-80 280v-40h-80q-33 0-56.5-23.5T200-280v-80h-40q-17 0-28.5-11.5T120-400q0-17 11.5-28.5T160-440h40v-80h-40q-17 0-28.5-11.5T120-560q0-17 11.5-28.5T160-600h40v-80q0-33 23.5-56.5T280-760h80v-40q0-17 11.5-28.5T400-840q17 0 28.5 11.5T440-800v40h80v-40q0-17 11.5-28.5T560-840q17 0 28.5 11.5T600-800v40h80q33 0 56.5 23.5T760-680v80h40q17 0 28.5 11.5T840-560q0 17-11.5 28.5T800-520h-40v80h40q17 0 28.5 11.5T840-400q0 17-11.5 28.5T800-360h-40v80q0 33-23.5 56.5T680-200h-80v40q0 17-11.5 28.5T560-120q-17 0-28.5-11.5T520-160v-40h-80v40q0 17-11.5 28.5T400-120q-17 0-28.5-11.5T360-160Zm320-120v-400H280v400h400ZM480-480Z"/> </svg>
    title: Easy to embed
    details: It takes only 25 lines of code to run the virtual machine. The virtual machine has no dependencies and supports 32-64 bit systems. 
  - icon: <svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 -960 960 960" width="24"><path id="feature-icon-path" d="M260-160q-91 0-155.5-63T40-377q0-78 47-139t123-78q25-92 100-149t170-57q117 0 198.5 81.5T760-520q69 8 114.5 59.5T920-340q0 75-52.5 127.5T740-160H260Zm0-80h480q42 0 71-29t29-71q0-42-29-71t-71-29h-60v-80q0-83-58.5-141.5T480-720q-83 0-141.5 58.5T280-520h-20q-58 0-99 41t-41 99q0 58 41 99t99 41Zm220-240Z"/></svg>
    title: Lightweight
    details: Simple syntax, simple codebase and compiled size less than 1MB.
  - icon: <svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 -960 960 960" width="24"><path id="feature-icon-path" d="M160-400q-33 0-56.5-23.5T80-480q0-33 23.5-56.5T160-560q33 0 56.5 23.5T240-480q0 33-23.5 56.5T160-400Zm38 144 118-118q11-11 28-11t28 11q11 11 11 28t-11 28L254-200q-11 11-28 11t-28-11q-11-11-11-28t11-28Zm120-332L200-706q-11-11-11-28t11-28q11-11 28-11t28 11l118 118q11 11 11 28t-11 28q-11 11-28 11t-28-11ZM480-80q-33 0-56.5-23.5T400-160q0-33 23.5-56.5T480-240q33 0 56.5 23.5T560-160q0 33-23.5 56.5T480-80Zm0-640q-33 0-56.5-23.5T400-800q0-33 23.5-56.5T480-880q33 0 56.5 23.5T560-800q0 33-23.5 56.5T480-720Zm106 76 120-118q11-11 27.5-11.5T762-762q11 11 11 28t-11 28L643-587q-12 12-28.5 12T586-587q-11-12-11.5-28.5T586-644Zm120 444L588-318q-11-11-11-28t11-28q11-11 28-11t28 11l118 118q11 11 11 28t-11 28q-11 11-28 11t-28-11Zm94-200q-33 0-56.5-23.5T720-480q0-33 23.5-56.5T800-560q33 0 56.5 23.5T880-480q0 33-23.5 56.5T800-400Z"/></svg>
    title: Coroutine based
    details: No threads. No multicore CPUs. Only coroutines. It can be used in single-threaded systems thanks to cooperative multitasking.
---

::: danger Caution!
The product is in the alpha version state. There may be bugs. In the next releases, something may change.
:::

<style>
#feature-icon-path {
    fill: var(--vp-c-brand-1);
}

:root {
    --vp-home-hero-image-filter: blur(44px);
    --vp-c-text-1: var(--vp-c-brand-1);
}

.dark {
    --vp-home-hero-image-background-image: radial-gradient(
        circle,
        #e4ddef52 100%,
        #e4ddef52 0%
    );
    
    --vp-home-hero-image-filter: blur(44px);
    
    --vp-c-text-1: var(--vp-c-brand-1);
}

@media (min-width: 640px) {
    :root {
        --vp-home-hero-image-filter: blur(56px);
    }
}

@media (min-width: 960px) {
    :root {
        --vp-home-hero-image-filter: blur(68px);
    }
}
</style>
