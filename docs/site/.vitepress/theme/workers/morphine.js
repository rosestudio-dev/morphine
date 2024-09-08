import Module from "../../generated/morphine.js";

const encoder = new TextEncoder();

const packageSize = 64

let outBuffer = []
let errBuffer = []

function sendBuffer(out) {
    const type = out ? "stdout" : "stderr"
    const buffer = out ? outBuffer : errBuffer
    postMessage({type, text: buffer.join("")})
}

function send(c, out) {
    const buffer = out ? outBuffer : errBuffer
    buffer.push(String.fromCharCode(c))

    if (buffer.length > packageSize) {
        sendBuffer(out)

        if (out) {
            outBuffer = []
        } else {
            errBuffer = []
        }
    }
}

function sendAll() {
    sendBuffer(true)
    sendBuffer(false)
    outBuffer = []
    errBuffer = []
}

async function call(code, stdin) {
    const module = await Module({
        stdout: (c) => {
            send(c, true)
        },
        stderr: (c) => {
            send(c, false)
        },
        stdin: () => {
            return stdin.shift()
        }
    })

    const codeArray = encoder.encode(code)
    module.ccall("morphine", "number", ["array", "number"], [codeArray, codeArray.length])
}

onmessage = function (e) {
    call(e.data.code, e.data.stdin).then(
        (value) => {
            sendAll()
            postMessage({type: "success", value})
        },
        (error) => {
            sendAll()
            postMessage({type: "error", error})
        }
    )
}
