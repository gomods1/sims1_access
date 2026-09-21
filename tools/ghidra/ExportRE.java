// Exports greppable text from an analysed Sims.exe dump into the directory given as the first
// script argument (default "re"):
//   functions.tsv     every function: va, name (with namespace), size, calling convention, prototype
//   vtables.tsv       every RTTI-labelled vftable: class, vtable va, slot, target va, target name
//   string_xrefs.tsv  every defined string: string va, referencing function va/name, text
//   import_xrefs.tsv  every imported API: dll::name, calling function va/name, call site va
// @category SimsRE

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;
import ghidra.program.model.symbol.SymbolType;

import java.io.File;
import java.io.PrintWriter;

public class ExportRE extends GhidraScript {

    private FunctionManager fm;

    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        File out = new File(args.length > 0 ? args[0] : "re");
        out.mkdirs();
        fm = currentProgram.getFunctionManager();
        exportFunctions(new File(out, "functions.tsv"));
        exportVtables(new File(out, "vtables.tsv"));
        exportStrings(new File(out, "string_xrefs.tsv"));
        exportImports(new File(out, "import_xrefs.tsv"));
        println("ExportRE done: " + out.getAbsolutePath());
    }

    private static String hex(Address a) {
        return String.format("%08x", a.getOffset());
    }

    private static String clean(String s) {
        return s.replace("\\", "\\\\").replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n");
    }

    private String owner(Address from) {
        Function f = fm.getFunctionContaining(from);
        return f == null ? "-\t-" : hex(f.getEntryPoint()) + "\t" + f.getName(true);
    }

    private void exportFunctions(File file) throws Exception {
        try (PrintWriter w = new PrintWriter(file, "UTF-8")) {
            w.println("va\tname\tsize\tconvention\tprototype");
            FunctionIterator it = fm.getFunctions(true);
            while (it.hasNext() && !monitor.isCancelled()) {
                Function f = it.next();
                w.printf("%s\t%s\t%d\t%s\t%s%n", hex(f.getEntryPoint()), f.getName(true),
                    f.getBody().getNumAddresses(), f.getCallingConventionName(),
                    clean(f.getPrototypeString(false, false)));
            }
        }
    }

    private void exportVtables(File file) throws Exception {
        SymbolTable st = currentProgram.getSymbolTable();
        Memory mem = currentProgram.getMemory();
        try (PrintWriter w = new PrintWriter(file, "UTF-8")) {
            w.println("class\tvtable_va\tslot\ttarget_va\ttarget_name");
            SymbolIterator it = st.getSymbols("vftable");
            while (it.hasNext() && !monitor.isCancelled()) {
                Symbol sym = it.next();
                Address vt = sym.getAddress();
                String cls = sym.getParentNamespace().getName(true);
                for (int slot = 0; slot < 512; slot++) {
                    Address entry = vt.add(slot * 4L);
                    if (slot > 0 && st.getPrimarySymbol(entry) != null
                            && st.getPrimarySymbol(entry).getSymbolType() != SymbolType.FUNCTION) {
                        break; // next vtable's meta pointer or another label
                    }
                    long target;
                    try {
                        target = mem.getInt(entry) & 0xffffffffL;
                    } catch (Exception e) {
                        break;
                    }
                    Address t = toAddr(target);
                    MemoryBlock b = mem.getBlock(t);
                    if (b == null || !b.isExecute()) {
                        break;
                    }
                    Function f = fm.getFunctionAt(t);
                    w.printf("%s\t%s\t%d\t%s\t%s%n", cls, hex(vt), slot, hex(t), f == null ? "" : f.getName(true));
                }
            }
        }
    }

    private void exportStrings(File file) throws Exception {
        try (PrintWriter w = new PrintWriter(file, "UTF-8")) {
            w.println("string_va\tfunc_va\tfunc_name\tref_va\ttext");
            DataIterator it = currentProgram.getListing().getDefinedData(true);
            while (it.hasNext() && !monitor.isCancelled()) {
                Data d = it.next();
                if (!d.hasStringValue()) {
                    continue;
                }
                Object v = d.getValue();
                String text = v == null ? "" : clean(v.toString());
                if (text.length() > 300) {
                    text = text.substring(0, 300) + "...";
                }
                Reference[] refs = getReferencesTo(d.getAddress());
                if (refs.length == 0) {
                    w.printf("%s\t-\t-\t-\t%s%n", hex(d.getAddress()), text);
                }
                for (Reference r : refs) {
                    w.printf("%s\t%s\t%s\t%s%n", hex(d.getAddress()), owner(r.getFromAddress()),
                        hex(r.getFromAddress()), text);
                }
            }
        }
    }

    private void exportImports(File file) throws Exception {
        SymbolTable st = currentProgram.getSymbolTable();
        try (PrintWriter w = new PrintWriter(file, "UTF-8")) {
            w.println("import\tfunc_va\tfunc_name\tsite_va");
            SymbolIterator it = st.getExternalSymbols();
            while (it.hasNext() && !monitor.isCancelled()) {
                Symbol ext = it.next();
                String name = ext.getParentNamespace().getName() + "::" + ext.getName();
                for (Reference toExt : ext.getReferences()) {
                    Address iat = toExt.getFromAddress();
                    boolean any = false;
                    for (Reference site : getReferencesTo(iat)) {
                        w.printf("%s\t%s\t%s%n", name, owner(site.getFromAddress()), hex(site.getFromAddress()));
                        any = true;
                    }
                    if (!any && fm.getFunctionContaining(iat) != null) {
                        w.printf("%s\t%s\t%s%n", name, owner(iat), hex(iat));
                    }
                }
            }
        }
    }
}
