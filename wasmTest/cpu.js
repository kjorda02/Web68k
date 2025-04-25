// COPIED RELEVANT CODE NECESSARY FOR PRINTF CALLS FROM EMSCRIPTEN-GENERATED 
// JS FILE WE USE INSTANTIATESTREAMING, WHICH IS FASTER AND ALLOWS US TO
// NOT HAVE TO DEFINE FUNCTION PROTOTYPES IN JS

// --------------------------------------------------------------
var __abort_js = () =>
    abort('native code called abort()');

// --------------------------------------------------------------
var exitJS = (status, implicit) => {
    EXITSTATUS = status;
    
    checkUnflushedContent();
    
    // if exit() was called explicitly, warn the user if the runtime isn't actually being shut down
    if (keepRuntimeAlive() && !implicit) {
        var msg = `program exited (with status: ${status}), but keepRuntimeAlive() is set (counter=${runtimeKeepaliveCounter}) due to an async operation, so halting execution but not exiting the runtime or preventing further async execution (you can use emscripten_force_exit, if you want to force a true shutdown)`;
        err(msg);
    }
    
    _proc_exit(status);
    };
    var _exit = exitJS;

// --------------------------------------------------------------
var _fd_close = (fd) => {
    abort('fd_close called without SYSCALLS_REQUIRE_FILESYSTEM');
};

// --------------------------------------------------------------
function _fd_seek(fd, offset, whence, newOffset) {
    abort('fd_seek called without SYSCALLS_REQUIRE_FILESYSTEM');
}

// --------------------------------------------------------------
var err = console.error.bind(console);
var out = console.log.bind(console);

function abort(what) {
  err('Aborted(' + what + ')');
  throw new WebAssembly.RuntimeError(what);
}

// --------------------------------------------------------------

var _fd_write = (fd, iov, iovcnt, pnum) => {
    let num = 0;
    let text = '';
    
    // Create a decoder once for all text
    const decoder = new TextDecoder();
    
    // Access the memory directly
    const buffer = cpu.memory.buffer;
    const view = new DataView(buffer);
    
    // Process all of the IOVs (I/O vectors)
    for (let i = 0; i < iovcnt; i++) {
        // Get pointer to the data and its length
        const ptr = view.getUint32(iov + i * 8, true);
        const len = view.getUint32(iov + i * 8 + 4, true);
        
        // Read the data as a string
        if (len > 0) {
        text += decoder.decode(new Uint8Array(buffer, ptr, len));
        num += len;
        }
    }
    
    // Output to the appropriate stream
    if (fd === 1) {
        console.log(text);
    } else if (fd === 2) {
        console.error(text);
    }
    
    // Write the number of bytes processed
    view.setUint32(pnum, num, true);
    return 0;
};

// --------------------------------------------------------------

// m68k ram takes 2^24 bytes. Each memory block is 2^16 bytes
var memory = new WebAssembly.Memory({ 
    initial: 512, // 2^25 / 2^16 = 2^9
    maximum: 1024 // 2^26 / 2^16 = 2^10
});

var wasmImports = {
    /** @export */
    _abort_js: __abort_js,
    /** @export */
    exit: _exit,
    /** @export */
    fd_close: _fd_close,
    /** @export */
    fd_seek: _fd_seek,
    /** @export */
    fd_write: _fd_write,
    emscripten_resize_heap: function(delta) {memory.grow(delta); }
  };


// END OF EMSCRIPTEN-GENERATED CODE
// --------------------------------------------------------------
var cpu;
WebAssembly.instantiateStreaming(
    fetch("../cpu.wasm"), {
        env: wasmImports,
        wasi_snapshot_preview1: wasmImports,
        js: {
            mem: memory
        }
    }
).then(results => {
  cpu = results.instance.exports;
  HEAPU32 = new Uint32Array(cpu.memory.buffer);
  HEAPU8 = new Uint8Array(cpu.memory.buffer);
  memory = results.instance.exports.memory;
});



document.getElementById('loadButton').addEventListener('click', function() {
    // Get the text from the text boxes
    const srec = document.getElementById('textBox1').value;
    const lst = document.getElementById('textBox2').value;

    cpu.initCpu();
    const ptr = cpu.wasm_malloc(srec.length +1); // +1 for sentinel null character at the end
    var arr = new Uint8Array(cpu.memory.buffer, ptr, srec.length +1);

    const bytes = (new TextEncoder()).encode(srec);
    arr.set(bytes);

    arr[srec.length] = 0; // sentinel
    cpu.load_program(ptr);

    cpu.wasmfree(ptr);
});