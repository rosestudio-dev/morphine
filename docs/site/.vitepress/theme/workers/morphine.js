import Module from "../../generated/morphine.js";

async function call(code, stdin) {
    const module = await Module({
        stdout: (c) => {
            postMessage({type: "stdout", text: String.fromCharCode(c)})
        },
        stderr: (c) => {
            postMessage({type: "stderr", text: String.fromCharCode(c)})
        },
        stdin: () => {
            return stdin.shift()
        }
    })

    module.ccall("morphine", "number", ["string", "number"], [code, code.length])
}

onmessage = function (e) {
    call(e.data.code, e.data.stdin).then(
        (value) => {
            postMessage({type: "success", value})
        },
        (error) => {
            postMessage({type: "error", error})
        }
    )
}
