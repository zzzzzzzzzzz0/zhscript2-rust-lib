use zhscript2::{u_::*, as_ref__, as_mut_ref__};
use std::fs::{File, Metadata};
use std::io::{self, Read, Write, Seek, SeekFrom};
use std::path::PathBuf;
use std::sync::mpsc::{Sender, channel};
use std::collections::hash_map::Entry;
use structopt::StructOpt;
use fnv::{FnvHashMap as HashMap, FnvHashSet as HashSet};
use blake3::Hasher;
use walkdir::{DirEntry, WalkDir};
#[cfg(unix)]
use std::os::unix::fs::MetadataExt;
use regex::Regex;
use glob::Pattern;

fn err(path: &PathBuf, err: io::Error) {
    eprintln!("Error processing file {}: {}", path.display(), err);
}

type HashSender = Sender<(u64, PathBuf, Vec<u8>)>;
type DupeSender = Sender<(u64, Vec<PathBuf>)>;

const BLOCKSIZE: usize = 4096;
const GAPSIZE: i64 = 102_400;

fn hash_file_inner(path: &PathBuf) -> io::Result<Vec<u8>> {
    let mut buf = [0u8; BLOCKSIZE];
    let mut fp = File::open(&path)?;
    let mut digest = Hasher::new();
    loop {
        match fp.read(&mut buf)? {
            0 => break,
            n => digest.update(&buf[..n]),
        };
        fp.seek(SeekFrom::Current(GAPSIZE))?;
    }
    Ok(digest.finalize().as_bytes().to_vec())
}

fn hash_file(verbose: bool, fsize: u64, path: PathBuf, tx: HashSender) {
    if verbose {
        let v = path.to_str().unwrap().to_string().as_bytes().to_vec();
        tx.send((fsize, path, v)).unwrap();
    } else {
        match hash_file_inner(&path) {
            Ok(hash) => tx.send((fsize, path, hash)).unwrap(),
            Err(e) => err(&path, e),
        }
    }
}

trait Candidate {
    fn read_block(&mut self) -> Result<usize, ()>;
    fn buf_equal(&self, other: &Self) -> bool;
    fn into_path(self) -> PathBuf;
}

struct FastCandidate {
    path: PathBuf,
    file: File,
    buf: [u8; BLOCKSIZE],
    n: usize,
}

struct SlowCandidate {
    path: PathBuf,
    pos: usize,
    buf: [u8; BLOCKSIZE],
    n: usize,
}

impl Candidate for FastCandidate {
    fn read_block(&mut self) -> Result<usize, ()> {
        match self.file.read(&mut self.buf) {
            Ok(n) => { self.n = n; Ok(n) },
            Err(e) => { err(&self.path, e); Err(()) }
        }
    }

    fn buf_equal(&self, other: &Self) -> bool {
        self.buf[..self.n] == other.buf[..other.n]
    }

    fn into_path(self) -> PathBuf {
        self.path
    }
}

impl Candidate for SlowCandidate {
    fn read_block(&mut self) -> Result<usize, ()> {
        match File::open(&self.path).and_then(|mut f| {
            f.seek(SeekFrom::Start(self.pos as u64)).and_then(|_| {
                f.read(&mut self.buf)
            })
        }) {
            Ok(n) => { self.n = n; self.pos += n; Ok(n) },
            Err(e) => { err(&self.path, e); Err(()) }
        }
    }

    fn buf_equal(&self, other: &Self) -> bool {
        self.buf[..self.n] == other.buf[..other.n]
    }

    fn into_path(self) -> PathBuf {
        self.path
    }
}

fn compare_files_inner<C: Candidate>(fsize: u64, mut todo: Vec<C>, tx: &DupeSender) {
    'outer: loop {
        let mut todo_diff = Vec::new();
        for i in (1..todo.len()).rev() {
            if !todo[i].buf_equal(&todo[0]) {
                todo_diff.push(todo.swap_remove(i));
            }
        }
        if todo_diff.len() >= 2 {
            compare_files_inner(fsize, todo_diff, tx);
        }
        if todo.len() < 2 {
            return;
        }
        for i in (0..todo.len()).rev() {
            match todo[i].read_block() {
                Ok(0) => break 'outer,
                Err(_) => { todo.remove(i); }
                _ => ()
            }
        }
    }
    tx.send((fsize, todo.into_iter().map(Candidate::into_path).collect())).unwrap();
}

fn compare_files(fsize: u64, paths: Vec<PathBuf>, tx: DupeSender) {
    if paths.len() < 100 {
        let todo = paths.into_iter().filter_map(|p| {
            match File::open(&p) {
                Ok(f) => Some(FastCandidate { path: p, file: f, buf: [0u8; BLOCKSIZE], n: 0 }),
                Err(e) => { err(&p, e); None }
            }
        }).collect();
        compare_files_inner(fsize, todo, &tx);
    } else {
        let todo = paths.into_iter().map(|p| {
            SlowCandidate { path: p, pos: 0, buf: [0u8; BLOCKSIZE], n: 0 }
        }).collect();
        compare_files_inner(fsize, todo, &tx);
    }
}

fn is_hidden_file(entry: &DirEntry) -> bool {
    entry.file_name()
        .to_str()
        .map(|s| s.starts_with("."))
        .unwrap_or(false)
}

#[derive(StructOpt)]
#[structopt(about="A parallel duplicate file finder.")]
struct Args {
    #[structopt(short="m", default_value="1", parse(try_from_str=unbytify::unbytify),
                help="Minimum file size to consider")]
    minsize: u64,
    #[structopt(short="M", parse(try_from_str=unbytify::unbytify),
                help="Maximum file size to consider")]
    maxsize: Option<u64>,
    #[structopt(short="H", help="Exclude Unix hidden files (names starting with dot)")]
    nohidden: bool,
    #[structopt(short="S", help="Don't scan recursively in directories?")]
    nonrecursive: bool,
    #[structopt(short="s", help="Report dupes on a single line?")]
    singleline: bool,
    #[structopt(short="v", help="路径当唯一")]
    verbose: bool,
    #[structopt(short="0", help="With -s, separate dupes with NUL, replace newline with two NULs")]
    nul: bool,
    #[structopt(short="f", help="Check only filenames matching this pattern", group="patterns")]
    pattern: Option<Pattern>,
    #[structopt(short="F", help="Check only filenames matching this regexp", group="patterns")]
    regexp: Option<Regex>,
    #[structopt(long="E", help="路径排除")]
    regexp2: Option<Regex>,
    #[structopt(help="Root directory or directories to search")]
    roots: Vec<PathBuf>,

    #[structopt(long="C")]
    code: Option<String>,
    #[structopt(long="C2", help="收集重复文件的代码")]
    code2: Option<String>,
    #[structopt(long="C3", help="Report a grand total of duplicates?")]
    code3: Option<String>,
    #[structopt(long="L", help="包括符号连接")]
    has_lnk: bool,
}

#[no_mangle]
pub extern fn fddf__(env:&code_::Env_) -> Result2_ {
    let Args { minsize, maxsize, verbose, singleline, nohidden,
        nonrecursive, nul, pattern, regexp, regexp2, roots,
        code, code2, code3, has_lnk,
    } = match Args::from_iter_safe({
        let q = as_ref__!(env.q);
        let a = as_ref__!(q.args_);
        a.to_vec__().iter()
    }) {
        Ok(o) => o,
        Err(err) => {
            let mut ret = as_mut_ref__!(env.ret);
            as_ref__!(env.w).dunhao__(&mut ret);
            ret.add__(err.to_string());
            return ok__();
        }
    };
    let maxsize = maxsize.unwrap_or(u64::max_value());

    enum Select {
        Pattern(Pattern),
        Regex(Regex),
        Any,
    }

    let select = if let Some(pat) = pattern {
        Select::Pattern(pat)
    } else if let Some(regex) = regexp {
        Select::Regex(regex)
    } else {
        Select::Any
    };

    let hidden_excluded = |entry: &DirEntry| nohidden && is_hidden_file(entry);

    let matches_pattern = |entry: &DirEntry| match select {
        Select::Any => true,
        Select::Pattern(ref p) => entry.file_name().to_str().map_or(false, |f| p.matches(f)),
        Select::Regex(ref r) => entry.file_name().to_str().map_or(false, |f| r.is_match(f)),
    };

    let mut sizes = HashMap::default();
    let mut hashes = HashMap::default();
    let mut inodes = HashSet::default();
    let mut paths = vec![];
    let mut paths1 = vec![];
    let mut roots2 = vec![];
    for root in &roots {
        roots2.push(root.to_string_lossy().to_string());
    }

    #[cfg(unix)]
    fn check_inode(set: &mut HashSet<(u64, u64)>, entry: &Metadata) -> bool {
        set.insert((entry.dev(), entry.ino()))
    }
    #[cfg(not(unix))]
    fn check_inode(_: &mut HashSet<(u64, u64)>, _: &Metadata) -> bool {
        true
    }

    let mut pool = scoped_threadpool::Pool::new(num_cpus::get() as u32 + 1);
    pool.scoped(|scope| {
        let (tx, rx) = channel();

        let hashref = &mut hashes;
        scope.execute(move || {
            for (size, path, hash) in rx.iter() {
                hashref.entry((size, hash)).or_insert_with(Vec::new).push(path);
            }
        });

        #[derive(Debug)]
        enum Found {
            One(PathBuf),
            Multiple
        }

        let mut process = |fsize, dir_entry: Option<DirEntry>, lnk:Option<String>, lnk2:&Option<(usize, PathBuf)>| {
            if dir_entry.is_none() {
                //paths1.push(paths.drain(..).collect());
                let mut v = vec![];
                v.append(&mut paths);
                paths1.push(v);
                return;
            }
            let path = dir_entry.unwrap().path().to_path_buf();
            if let Some((i, pb)) = lnk2 {
                let path1 = path.to_string_lossy().to_string();
                let path2 = pb.to_string_lossy().to_string() + &path1[*i..];
                paths.push((PathBuf::from(path2), fsize, Some(path1)));
                return;
            }
            let is_lnk = lnk.is_some();
            paths.push((path.clone(), fsize, lnk));
            if is_lnk {
                return;
            }
            match sizes.entry(fsize) {
                Entry::Vacant(v) => {
                    v.insert(Found::One(path));
                }
                Entry::Occupied(mut v) => {
                    let first = std::mem::replace(v.get_mut(), Found::Multiple);
                    if let Found::One(first_path) = first {
                        let txc = tx.clone();
                        scope.execute(move || hash_file(verbose, fsize, first_path, txc));
                    }
                    let txc = tx.clone();
                    scope.execute(move || hash_file(verbose, fsize, path, txc));
                }
            }
        };

        fn for__<F1, F2, F3>(walkdir:WalkDir, regexp2: &Option<Regex>, minsize:u64, maxsize:u64,
                has_lnk:bool, lnk2:&Option<(usize, PathBuf)>,
                inc:&F1, check_inode:&mut F2, process:&mut F3)
                where
                F1:Fn(&DirEntry) -> bool,
                F2:FnMut(&Metadata) -> bool,
                F3:FnMut(u64, Option<DirEntry>, Option<String>, &Option<(usize, PathBuf)>),
        {
            for dir_entry in walkdir {
                match dir_entry {
                    Ok(dir_entry) => {
                        let ft = dir_entry.file_type();
                        let ex = if let Some(r) = regexp2 {
                            let f = dir_entry.path().to_str().unwrap();
                            r.is_match(&f)
                        } else {false};
                        if !ex {
                            if ft.is_file() {
                                match dir_entry.metadata() {
                                    Ok(meta) => {
                                        let fsize = meta.len();
                                        if fsize >= minsize && fsize <= maxsize {
                                            if check_inode(&meta) {
                                                if inc(&dir_entry) {
                                                    process(fsize, Some(dir_entry), None, lnk2);
                                                }
                                            }
                                        }
                                    }
                                    Err(e) => {
                                        eprintln!("{}", e);
                                    }
                                }
                            }
                            else if has_lnk && ft.is_symlink() {
                                let pb0 = dir_entry.path().to_path_buf();
                                let mut s = String::new();
                                let mut pb = pb0.clone();
                                loop {
                                    if pb.is_symlink() {
                                        if let Ok(pb2) = pb.read_link() {
                                            s = pb2.to_str().unwrap().to_string();
                                            if pb2.is_relative() {
                                                if let Some(pb3) = pb.parent() {
                                                    s.insert_str(0, &(pb3.to_str().unwrap().to_string() + "/"));
                                                }
                                            }
                                        } else {
                                            break;
                                        }
                                    } else {
                                        if pb.exists() {
                                            s = pb.canonicalize().unwrap().to_str().unwrap().to_string();
                                        } else {
                                            s.clear()
                                        }
                                        break;
                                    }
                                    pb = PathBuf::from(&s);
                                }
                                if pb.is_dir() {
                                    for__(WalkDir::new(&s).follow_links(false), regexp2, minsize, maxsize,
                                        has_lnk, &Some((s.len(), pb0.clone())),
                                        inc, check_inode, process);
                                } else {
                                    if !inc(&dir_entry) {
                                        continue;
                                    }
                                    process(0, Some(dir_entry), Some(s), lnk2);
                                }
                            }
                        }
                    }
                    Err(_e) => {}
                }
            }
            process(0, None, None, lnk2);
        }
        let roots = if roots.is_empty() { vec![".".into()] } else { roots };
        for root in roots {
            let walkdir = if nonrecursive {
                WalkDir::new(root).max_depth(1).follow_links(false)
            } else {
                WalkDir::new(root).follow_links(false)
            };
            for__(walkdir, &regexp2, minsize, maxsize, has_lnk, &None,
                &|dir_entry| !hidden_excluded(dir_entry) && matches_pattern(dir_entry),
                &mut |meta| check_inode(&mut inodes, meta),
                &mut |fsize, dir_entry, lnk, lnk2| process(fsize, dir_entry, lnk, lnk2));
        }
    });

    let mut total_dupes = 0;
    let mut total_files = 0;
    let mut total_size = 0;
    let mut paths2 = vec![];

    {
        let mut print_dupe = |out: &mut io::StdoutLock, size, entries: Vec<PathBuf>| {
            total_dupes += 1;
            total_files += entries.len() - 1;
            total_size += size * (entries.len() - 1) as u64;
            if singleline {
                let last = entries.len() - 1;
                for (i, path) in entries.into_iter().enumerate() {
                    write!(out, "{}", path.display()).unwrap();
                    if i < last {
                        if nul {
                            write!(out, "\0").unwrap();
                        } else {
                            write!(out, " ").unwrap();
                        }
                    }
                }

                if nul {
                    write!(out, "\0\0").unwrap();
                } else {
                    writeln!(out).unwrap();
                }
            } else {
                let mut i1 = String::new();
                for path in entries {
                    for paths in &mut paths1 {
                        if let Some(idx) = paths.iter().position(|i| i.0.eq(&path)) {
                            if i1.is_empty() {
                                i1.push_str(&paths[idx].0.to_string_lossy());
                            } else {
                                paths2.push((paths.remove(idx), i1.clone()));
                            }
                        }
                    }
                }
            }
        };

        pool.scoped(|scope| {
            let (tx, rx) = channel();

            scope.execute(move || {
                let stdout = io::stdout();
                let mut stdout = stdout.lock();
                for (size, entries) in rx.iter() {
                    print_dupe(&mut stdout, size, entries);
                }
            });

            for ((fsize, _), entries) in hashes {
                if entries.len() > 1 {
                    let txc = tx.clone();
                    scope.execute(move || compare_files(fsize, entries, txc));
                }
            }
        });
    }

    {
        let mut v = vec![];
        for paths in &paths1 {
            let mut v2 = paths.iter()
                .filter(|i| if let Some(s) = &i.2 {!s.is_empty()} else {false})
                .map(|i| (i.0.to_string_lossy().to_string(), PathBuf::from(i.2.clone().unwrap())))
                .collect::<Vec<(String, PathBuf)>>();
            v.append(&mut v2);
        }
        v.dedup();
        //println!("{:?}", v);
        for path in v {
            for paths in &mut paths1 {
                if let Some(idx) = paths.iter().position(|i| i.0.eq(&path.1)) {
                    paths2.push((paths.remove(idx), path.0.clone()));
                }
            }
        }
    }
    if let Some(code) = code {
        let mut only_b = false;
        'l: for paths in &mut paths1 {
            paths.sort();
            /*for i in &paths {
                println!("{:?}", i);
            }*/
            for i in paths {
                let q = Qv_::new2(Some(env.q.clone()));
                {
                    let args = &mut as_mut_ref__!(q.args_);
                    let path = i.0.to_string_lossy();
                    let mut name = path.to_string();
                    for root in &roots2 {
                        if name.starts_with(root) {
                            let start = root.len() + if root.ends_with('/') {0} else {1};
                            if start < name.len() {
                                name = name[start..].to_string();
                            } else {
                                name.clear();
                            }
                        }
                    }
                    args.add__(name);
                    as_ref__!(env.w).dunhao__(args);
                    args.add__(path);
                    if let Some(s) = &i.2 {
                        as_ref__!(env.w).dunhao__(args);
                        args.add__(s);
                    }
                }
                let ret = eval_::hello__(&code, &mut code_::Env_::new2(t__(q), env));
                jump_::for_err2__(ret, &mut only_b)?;
                if only_b {break 'l;}
            }
        }
    }
    if let Some(code) = code2 {
        let mut only_b = false;
        for i in &paths2 {
            let q = Qv_::new2(Some(env.q.clone()));
            {
                let args = &mut as_mut_ref__!(q.args_);
                args.add__(i.0.0.to_string_lossy());
                as_ref__!(env.w).dunhao__(args);
                args.add__(i.1.clone());
            }
            let ret = eval_::hello__(&code, &mut code_::Env_::new2(t__(q), env));
            jump_::for_err2__(ret, &mut only_b)?;
            if only_b {break;}
        }
    }
    if let Some(code) = code3 {
        let q = Qv_::new2(Some(env.q.clone()));
        {
            let args = &mut as_mut_ref__!(q.args_);
            {
                let mut len = 0;
                for paths in &mut paths1 {
                    len += paths.len();
                }
                args.add__(len);
            }
            as_ref__!(env.w).dunhao__(args);
            args.add__(total_dupes);
            as_ref__!(env.w).dunhao__(args);
            args.add__("groups of duplicate files");
            as_ref__!(env.w).dunhao__(args);
            args.add__(total_files);
            as_ref__!(env.w).dunhao__(args);
            args.add__("files are duplicates");
 
            let (val, suffix) = unbytify::bytify(total_size);
            as_ref__!(env.w).dunhao__(args);
            args.add__(val);
            as_ref__!(env.w).dunhao__(args);
            args.add__(suffix);
            as_ref__!(env.w).dunhao__(args);
            args.add__("of space taken by duplicates");
        }
        eval_::hello__(&code, &mut code_::Env_::new2(t__(q), env))?;
    }

    ok__()
}

