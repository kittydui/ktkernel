use clap::Parser;
use std::path::{Path, PathBuf};
use std::process::Command;

#[derive(Parser)]
#[command(name = "modulec", about = "Kernel module compiler for ktOS")]
struct Args {
    /// Source files to compile
    #[arg(long, required = true, num_args = 1..)]
    srcs: Vec<PathBuf>,

    /// Include directories
    #[arg(long, num_args = 1..)]
    includes: Vec<PathBuf>,

    /// Target triple (e.g. x86_64-elf)
    #[arg(long, default_value = "x86_64-elf")]
    target: String,

    /// Additional compiler flags
    #[arg(long, num_args = 1..)]
    compile_flags: Vec<String>,

    /// Output file path (without extension)
    #[arg(short, long, required = true)]
    output: PathBuf,

    /// Write compile_commands.json fragment to this path
    #[arg(long)]
    compdb: Option<PathBuf>,
}

fn main() {
    let args = Args::parse();

    if let Err(e) = build_module(&args) {
        eprintln!("modulec: error: {e}");
        std::process::exit(1);
    }
}

fn build_module(args: &Args) -> Result<(), String> {
    let tmp_dir = std::env::temp_dir().join(format!("modulec_{}", std::process::id()));
    std::fs::create_dir_all(&tmp_dir)
        .map_err(|e| format!("failed to create temp dir: {e}"))?;

    let result = compile_and_link(args, &tmp_dir);

    let _ = std::fs::remove_dir_all(&tmp_dir);
    result
}

fn compile_and_link(args: &Args, tmp_dir: &Path) -> Result<(), String> {
    let mut objects: Vec<PathBuf> = Vec::new();
    let mut compdb_entries: Vec<CompDbEntry> = Vec::new();

    for src in &args.srcs {
        let (obj, entry) = compile_source(src, args, tmp_dir)?;
        objects.push(obj);
        compdb_entries.push(entry);
    }

    let output = args.output.with_extension("ktdrv");

    if objects.len() == 1 {
        std::fs::copy(&objects[0], &output)
            .map_err(|e| format!("failed to copy object to output: {e}"))?;
    } else {
        partial_link(&objects, &output)?;
    }

    if let Some(compdb_path) = &args.compdb {
        write_compdb(compdb_path, &compdb_entries)?;
    }

    println!("modulec: built {}", output.display());
    Ok(())
}

struct CompDbEntry {
    directory: String,
    file: String,
    arguments: Vec<String>,
}

fn build_compiler_args(src: &Path, obj_path: &Path, args: &Args) -> Vec<String> {
    let mut compiler_args = vec![
        "clang++".into(),
        "-c".into(),
        "-o".into(), obj_path.display().to_string(),
        src.display().to_string(),
        format!("--target={}", args.target),
        "-ffreestanding".into(),
        "-fno-builtin".into(),
        "-fno-exceptions".into(),
        "-fno-rtti".into(),
        "-fno-pic".into(),
        "-fno-stack-protector".into(),
        "-mno-red-zone".into(),
        "-mcmodel=kernel".into(),
    ];

    for inc in &args.includes {
        compiler_args.push(format!("-I{}", inc.display()));
    }

    for flag in &args.compile_flags {
        compiler_args.push(flag.clone());
    }

    compiler_args
}

fn compile_source(src: &Path, args: &Args, tmp_dir: &Path) -> Result<(PathBuf, CompDbEntry), String> {
    let stem = src.file_stem()
        .ok_or_else(|| format!("invalid source file: {}", src.display()))?;

    let obj_path = tmp_dir.join(format!("{}.o", stem.to_string_lossy()));
    let compiler_args = build_compiler_args(src, &obj_path, args);

    let cwd = std::env::current_dir()
        .map_err(|e| format!("failed to get cwd: {e}"))?;

    let src_abs = if src.is_absolute() {
        src.to_path_buf()
    } else {
        cwd.join(src)
    };

    let entry = CompDbEntry {
        directory: cwd.display().to_string(),
        file: src_abs.display().to_string(),
        arguments: compiler_args.clone(),
    };

    let mut cmd = Command::new(&compiler_args[0]);
    for arg in &compiler_args[1..] {
        cmd.arg(arg);
    }

    let output = cmd.output()
        .map_err(|e| format!("failed to run clang++: {e}"))?;

    if !output.status.success() {
        let stderr = String::from_utf8_lossy(&output.stderr);
        return Err(format!("compilation failed for {}:\n{stderr}", src.display()));
    }

    Ok((obj_path, entry))
}

fn write_compdb(path: &Path, entries: &[CompDbEntry]) -> Result<(), String> {
    let json_entries: Vec<String> = entries.iter().map(|e| {
        let args_json: Vec<String> = e.arguments.iter()
            .map(|a| format!("\"{}\"", a.replace('\\', "\\\\").replace('"', "\\\"")))
            .collect();

        format!(
            "  {{\n    \"directory\": \"{}\",\n    \"file\": \"{}\",\n    \"arguments\": [{}]\n  }}",
            e.directory.replace('\\', "\\\\").replace('"', "\\\""),
            e.file.replace('\\', "\\\\").replace('"', "\\\""),
            args_json.join(", "),
        )
    }).collect();

    let json = format!("[\n{}\n]\n", json_entries.join(",\n"));

    std::fs::write(path, json)
        .map_err(|e| format!("failed to write compdb: {e}"))?;

    Ok(())
}

fn partial_link(objects: &[PathBuf], output: &Path) -> Result<(), String> {
    let mut cmd = Command::new("ld.lld");
    cmd.arg("-r").arg("-o").arg(output);

    for obj in objects {
        cmd.arg(obj);
    }

    let result = cmd.output()
        .map_err(|e| format!("failed to run ld.lld: {e}"))?;

    if !result.status.success() {
        let stderr = String::from_utf8_lossy(&result.stderr);
        return Err(format!("partial link failed:\n{stderr}"));
    }

    Ok(())
}
