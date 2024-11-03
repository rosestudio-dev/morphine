![Deploy Docs](https://github.com/rosestudio-dev/morphine-lang/actions/workflows/deploy-docs.yml/badge.svg)

# Morphine Programming Language
![logo](extra/images/morphine-logo-light.png#gh-light-mode-only)
![logo](extra/images/morphine-logo-dark.png#gh-dark-mode-only)
Morphine is an open-source, embedded programming language developed by [why-iskra](https://github.com/why-iskra).

> [!CAUTION]
> The product is in the dev version state. There may be bugs. In the next releases, something may change.

## Contents
- [Ideology](#ideology)
- [Features](#features)
- [Code looks](#code-looks)
- [Try It Now](#try-it-now)
- [Building from source](#building-from-source)
- [License](#license)

## Ideology
Morphine has the following main features that build the language's ideology:
- Easy to embed
- Lightweight
- Coroutine based

## Features
- Strong dynamic typing
- Object-oriented (prototype-based)
- Garbage collector
- Explicit control of functions behavior (recursion, closures)
- Coroutines (cooperative multitasking at the virtual machine level)
- Powerful embedding api
- Lightweight (binaries is less than 1MB)
- No dependencies (for the virtual machine and the compiler; excluding stdlib)
- Designed for embedded systems

## Code looks
```
val {
    println,
    setmetatable,
    error,
    pcall
} = env.library("base")

val tostr = env.library("value.tostr")
val getallocated = env.library("gc.getallocated")
val format = env.library("string.format")
val coroutine = env.library("coroutine")
val vector = env.library("vector")
val exception = env.library("exception")
val sio = env.library("sio")

fun dump<println>() println(self) end

fun getobject<auto>() = setmetatable({ index = 0, dump }) {
    _mf_gc = fun<println, format>()
        println(format("collected (index: ${index})!", self))
        leave
        println("Unreached ...")
    end,
    _mf_add = fun(value)
        self.index += value or return "oh no..."
    end
}

fun worker<auto>()
    val current = coroutine.current()
    val name = coroutine.name(current)
    println("Coroutine " .. name .. " started")

    fun recursive getting(value) = if(value > 0)
        invoked(value - 1)
    else
        self
    end

    val text = format("Recursion allocated: ${value}KB") {
        value = do
            val allocated = getallocated()
            getting!allocated(100) / 1024
        end
    }

    println(text)

    val object = getobject()
    for(var i = 0; i < 1000; i++)
        eval object + i
        yield
    end
    println(object + nil)
    object:dump()
end

do
    val coroutines = vector.unfixed([])
    for(var i = 0; i < 100; i++)
        val created = coroutine.create("worker" .. tostr(i), worker)
        vector.push(coroutines, created)
    end

    iterator({ key, value } in coroutines)
        coroutine.launch(value)
        coroutines[key] = nil
    end
end

// Reference test
do
    val value = ref { text = "reference test" }
    env.library("gc").full()
    println(*value or "cleared")
end

/*
 * Protected call
 */

fun pcalltest<error>(throw) = throw or error("hello world!")

do
    var pcallres = pcall(pcalltest, nil)
    println(exception.value(pcallres))
    exception.print(pcallres, sio.io())
end

do
    val pcallres = pcall(pcalltest, "no errors")
    println(pcallres)
end
```

## Try It Now
You can try [morphine language](https://rosestudio-dev.github.io/morphine-lang/playground) on your browser.
> [!NOTE]
> Some libraries are disabled, memory is limited, runtime may be slowed down because this is the WebAssembly version of the language.

## Building from source
### Software requirements
- python3
- meson
- ninja
- cc
### Commands
```
meson setup buildDir
meson install -C buildDir
```

## License
> [!NOTE]
> This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.