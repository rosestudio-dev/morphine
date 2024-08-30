import './tailwind.postcss'
import type { Theme } from 'vitepress'
import DefaultTheme from 'vitepress/theme'
import './custom.css'
import Layout from "./components/Layout.vue"
import MorphineCode from "./components/MorphineCode.vue"
import MorphinePlayground from "./components/MorphinePlayground.vue"
import Console from "./components/Console.vue"

export default {
    extends: DefaultTheme,
    enhanceApp({ app }) {
        app.component("Console", Console)
        app.component("MorphineCode", MorphineCode)
        app.component("MorphinePlayground", MorphinePlayground)
    },
    Layout: Layout
} satisfies Theme