//import Module from '../vasmm68k_motvasm.js';

// Wait for the module to be initialized before using it
Module.onRuntimeInitialized = function() {
    console.log('WebAssembly module initialized, now running assembler...');
    const result = performAssembly();
};

function performAssembly() {
    // Run the assembler
    const args = ['-m68000', '-chklabels', '-chklabels', '-Fsrec', '-exec=START', '-o', '/dev/stdout'];
    

    //Module.FS.init();
    const exitCode = Module.callMain(args);


    return exitCode;
}

// Don't call performAssembly() directly - it will be called after initialization
console.log('Waiting for WebAssembly module to initialize...');