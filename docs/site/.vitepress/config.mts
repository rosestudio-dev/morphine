import {defineConfig} from 'vitepress'

export default defineConfig({
    base: '/morphine-lang/',
    title: "Morphine Language",
    lastUpdated: true,
    themeConfig: {
        sidebar: {
            '/vm/': {base: '/vm/', items: sidebarVm()},
        },

        logo: {
            light: '/assets/logo-small-light.svg',
            dark: '/assets/logo-small-dark.svg'
        },

        nav: [
            {text: 'Home', link: '/'},
            {text: 'Quickstart', link: '/quickstart'},
            {text: 'Language', link: '/language'},
            {text: 'Virtual machine', link: '/vm/getting-started'},
            {text: 'Compiler', link: '/compiler'},
            {text: 'Changelog', link: '/changelog'},
        ],

        socialLinks: [
            {icon: 'github', link: 'https://github.com/rosestudio-dev/morphine-lang'}
        ],

        footer: {
            message: 'Released under the MIT License',
            copyright: 'Copyright © 2024-present <a href="https://github.com/why-iskra">why-iskra</a>'
        },

        search: {
            provider: 'local'
        }
    },
    head: [
        [
            'link',
            {
                rel: 'icon',
                type: 'image/svg+xml',
                href: 'assets/logo-small-dark.svg',
                media: '(prefers-color-scheme:dark)'
            }
        ],
        [
            'link',
            {
                rel: 'icon',
                type: 'image/svg+xml',
                href: 'assets/logo-small-light.svg',
                media: '(prefers-color-scheme:light)'
            }
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
    ],
    rewrites: {
        'generated/:path(.*)': ':path'
    }
})

function sidebarVm(): DefaultTheme.SidebarItem[] {
    return [
        {
            text: 'Introduction',
            collapsed: false,
            items: [
                {text: 'Getting Started', link: 'getting-started'},
            ]
        },
        {
            text: 'Basic',
            collapsed: false,
            items: [
                {text: 'Instance', link: 'instance'},
                {text: 'Values and objects', link: 'values-and-objects'},
                {text: 'Coroutines', link: 'coroutines'},
                {text: 'Call', link: 'call'},
                {text: 'Interpreter', link: 'interpreter'},
                {text: 'Garbage Collector', link: 'garbage-collector'},
                {text: 'Require Function', link: 'require-function'},
            ]
        },
        {
            text: 'API',
            collapsed: false,
            items: [
                {text: 'Allocator', link: 'api-allocator'},
                {text: 'Callstack', link: 'api-callstack'},
                {text: 'Coroutine', link: 'api-coroutine'},
                {text: 'Garbage collector', link: 'api-garbage-collector'},
                {text: 'Instance', link: 'api-instance'},
                {text: 'Iterator', link: 'api-iterator'},
                {text: 'Version', link: 'api-version'},
            ]
        },
    ]
}
