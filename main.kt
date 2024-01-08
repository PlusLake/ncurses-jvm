import java.io.OutputStream
import java.net.*
import java.nio.*
import java.nio.file.*
import java.nio.channels.*

val PATH = Path.of("/tmp/kncurses-socket")

fun main() {
    channel().also { Thread { socket(it, render()) }.start() }
    ProcessBuilder("./jcurses")
        .also(ProcessBuilder::inheritIO)
        .start()
        .waitFor()
        .also { println("Exit code of jcurses: $it") }
}

fun render() : Render {
    val array = ShortArray(20)
    var cursor = 0
    return fun(signal, display) {
        Thread.sleep(1)
        if (signal.key.toInt().toChar() == 'q') {
            display.exit()
            System.exit(0)
        }
        display.clear()
        display.move(signal.width - 10, signal.height - 1)
        display.print("${signal.width}, ${signal.height}")

        if (signal.key.toInt() != 0) array[cursor % array.size] = signal.key
        for (i in array.indices) {
            display.move(signal.width - 6, i)
            display.print("${array[i]}")
        }
        if (signal.key.toInt() == 0) return
        display.move(signal.width - 8, cursor++ % array.size)
        display.print(">")
        display.move(signal.width.toInt(), signal.height.toInt() - 1)
    }
}

fun channel() : ServerSocketChannel {
    Files.deleteIfExists(PATH)
    return ServerSocketChannel
        .open(StandardProtocolFamily.UNIX)
        .also { it.bind(UnixDomainSocketAddress.of(PATH)) }
}

fun socket(channel: ServerSocketChannel, callback: Render) = with (channel.accept()) {
    val stream = Channels.newInputStream(this)
    val display = Display(Channels.newOutputStream(this))
    while (true) {
        val bytes = stream.readNBytes(6)
        if (bytes.isEmpty()) break

        ByteBuffer
            .wrap(bytes)
            .order(ByteOrder.LITTLE_ENDIAN)
            .let { Signal(it.short, it.short, it.short) }
            .also { callback(it, display) }
    }
}

class Display(val out: OutputStream) {
    fun exit() = out.write(byteArrayOf(0))
    fun print(string: String) = string
        .toByteArray()
        .let {
            ByteBuffer
                .allocate(it.size + 5)
                .order(ByteOrder.LITTLE_ENDIAN)
                .put(1)
                .putInt(it.size)
                .put(it)
        }
        .array()
        .also(out::write)
    fun move(x: Int, y: Int) = ByteBuffer
        .allocate(5)
        .order(ByteOrder.LITTLE_ENDIAN)
        .put(2)
        .putShort(x.toShort())
        .putShort(y.toShort())
        .array()
        .also(out::write)
    fun clear() = out.write(byteArrayOf(3))
}

data class Signal(val key: Short, val width: Short, val height: Short)
typealias Render = (Signal, Display) -> Unit
