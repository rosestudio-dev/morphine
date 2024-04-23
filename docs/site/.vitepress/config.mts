import {defineConfig} from 'vitepress'

export default defineConfig({
    base: '/morphine-lang/',
    title: "Morphine Language",
    lastUpdated: true,
    themeConfig: {
        logo: {
            light: '/assets/logo-small-light.svg',
            dark: '/assets/logo-small-dark.svg'
        },

        nav: [
            {text: 'Home', link: '/'},
            {text: 'Quickstart', link: '/quickstart'},
            {text: 'Language', link: '/language'},
            {text: 'Virtual machine', link: '/vm'},
            {text: 'Compiler', link: '/compiler'},
            {text: 'Examples', link: '/examples'},
            {text: 'Changelog', link: '/changelog'},
        ],

        socialLinks: [
            {icon: 'github', link: 'https://github.com/rosestudio-dev/morphine-lang'}
        ],

        footer: {
            message: 'Released under the MIT License.',
            copyright: 'Copyright © 2024-present <a href="https://github.com/why-iskra">why-iskra</a>'
        }
    },
    head: [
        [
            'link',
            {rel: 'icon', type: 'image/svg+xml', href: 'assets/logo-small-dark.svg', media: '(prefers-color-scheme:dark)'}
        ],
        [
            'link',
            {rel: 'icon', type: 'image/svg+xml', href: 'assets/logo-small-light.svg', media: '(prefers-color-scheme:light)'}
        ],
        [
            'link',
            {rel: 'preconnect', href: 'https://fonts.googleapis.com'}
        ],
        [
            'link',
            {rel: 'preconnect', href: 'https://fonts.gstatic.com', crossorigin: ''}
        ],
        [
            'link',
            {
                href: 'https://fonts.googleapis.com/css2?family=Material+Symbols+Rounded:opsz,wght,FILL,GRAD@24,400,0,0',
                rel: 'stylesheet'
            }
        ]
    ]
})
