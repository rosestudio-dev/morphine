/*
 * Mandelbrot set
 */

val base = env.lib("base")
val chars = [" ", ".", ":", "-", "=", "+", "*", "#", "%", "@"]
for(var y = -1.3; y <= 1.3; y += 0.3) {
    for(var x = -2.1; x <= 1.1; x += 0.1) {
        var zi = 0.0
        var zr = 0.0
        var i = 0
        while(i < 100) {
            if (zi * zi + zr * zr >= 4.0) { break }
            val rt = zr
            zr = zr * zr - zi * zi + x
            zi = 2.0 * rt * zi + y
            i ++
        }
        base.print(chars[i % 10])
    }
    base.print("\n")
}
