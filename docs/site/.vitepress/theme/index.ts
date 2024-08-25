import type { Theme } from 'vitepress'
import DefaultTheme from 'vitepress/theme'
import './custom.css'
import Layout from "./components/Layout.vue"

export default {
    extends: DefaultTheme,
    enhanceApp({ app }) {
        // app.component("MorphinePlayground", MorphinePlayground)
    },
    Layout: Layout
} satisfies Theme