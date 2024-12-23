do {
    val app = env.lib("__app")
    val compiler = env.lib("compiler")

    env.load = fun load<auto>(name) {
        env.__load_cache = env.__load_cache or { }

        val result = env.__load_cache[name] or do {
            val script = compiler.compile {
                text = app.getscript(name),
                name = name
            }

            script()
        }

        env.__load_cache[name] = result
        return result
    }

    env.private_libs = ["__app", "closure"]
}

env.load("launcher")
