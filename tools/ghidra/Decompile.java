// Decompiles the functions containing the given addresses into <outDir>/<va>_<name>.c.
// Args: <outDir> <va> [va ...]   (hex, preferred-base VAs)
// @category SimsRE

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;

import java.io.File;
import java.io.PrintWriter;

public class Decompile extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        File out = new File(args[0]);
        out.mkdirs();
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        for (int i = 1; i < args.length; i++) {
            Function f = getFunctionContaining(toAddr(Long.parseLong(args[i].replace("0x", ""), 16)));
            if (f == null) {
                println("no function at " + args[i]);
                continue;
            }
            DecompileResults r = di.decompileFunction(f, 120, monitor);
            String name = f.getName(true).replaceAll("[^A-Za-z0-9_]", "_");
            File file = new File(out, String.format("%08x_%s.c", f.getEntryPoint().getOffset(), name));
            try (PrintWriter w = new PrintWriter(file, "UTF-8")) {
                w.println("// " + f.getName(true) + " @ " + f.getEntryPoint() + " (requested " + args[i] + ")");
                w.println(r.decompileCompleted() ? r.getDecompiledFunction().getC() : "// FAILED: " + r.getErrorMessage());
            }
            println("wrote " + file.getName());
        }
        di.dispose();
    }
}
