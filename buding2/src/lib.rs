#![feature(default_free_fn)]

use zhscript2::{u_::*, as_ref__, as_mut_ref__};
use std::{fs::{self, File}, io::prelude::*, default};
use base64::{Engine as _, engine::general_purpose};
#[cfg(feature = "re")]
use regex::Regex;

mod socket_;
mod t_;
use t_::*;
mod cmp_;

#[no_mangle]
pub extern fn dir__(env:&code_::Env_) -> Result2_ {
	let q = as_ref__!(env.q);
	let args = {
		let args = as_ref__!(q.args_);
		args.to_vec__()
	};
	if_buzu__(args.len(), 2, usize::MAX)?;
	/*if cfg!(debug_assertions) {
		println!("{:?}", args)
	}*/

	#[allow(non_snake_case)]
	#[derive(Default, Debug)]
	struct Opt_ {
		s_:bool,
		a_d_:bool,
		a_f_:bool,
		a_h_:bool,
		a_l_:bool,
		ah_:bool,
		n_:bool,
		f_:bool,
		P_:bool,
		#[cfg(feature = "re")]
		match_:Option<Regex>,
	}
	let mut o:Opt_ = default::default();
	if args.len() > 2 {
		let s = &args[2];
		if !s.is_empty() {
			#[cfg(feature = "re")]
			match Regex::new(&args[2]) {
				Ok(re) => o.match_ = Some(re),
				Err(err) => return result2_::err__(err.to_string())
			}
			#[cfg(not(feature = "re"))]
			return result2_::err__([s, " 不支持处理"].concat())
		}
	}
	if args.len() > 3 {
		let opt = &args[3];
		let mut c2 = ' ';
		for c in opt.chars() {
			let no_in__ = |s2:&str| {
				result2_::err__(["设项 ".to_string(), c.to_string(), " 不在 ".to_string(), s2.to_string(), " 之列".to_string()].concat())
			};
			let mut b = false;
			match c2 {
				'a' => {
					match c {
						'-' => {
							c2 = c;
							continue;
						}
						'h' => {
							o.ah_ = true;
							b = true;
						}
						_ => return no_in__("-、h")
					}
				}
				'-' => {
					match c {
						'd' => o.a_d_ = true,
						'f' => o.a_f_ = true,
						'h' => o.a_h_ = true,
						'l' => o.a_l_ = true,
						_ => return no_in__("d、f、h、l")
					}
					b = true;
				}
				_ => {}
			}
			if b {
				c2 = ' ';
				continue;
			}
			match c {
				's' => o.s_ = true,
				'a' => c2 = c,
				'n' => o.n_ = true,
				'f' => o.f_ = true,
				'P' => o.P_ = true,
				' ' => {}
				_ => return no_in__("s、a、n、f、P")
			}
		}
	}

	use std::cmp::Ordering;
	#[derive(Eq)]
	struct Item_ {
		title_:String,
		path_:String,
		typ_:String,
	}
	impl Ord for Item_ {
		fn cmp(&self, other: &Self) -> Ordering {
			cmp_::bb__(self.title_.as_bytes(), other.title_.as_bytes())
		}
	}
	impl PartialOrd for Item_ {fn partial_cmp(&self, other: &Self) -> Option<Ordering> {Some(self.cmp(other))}}
	impl PartialEq for Item_ {fn eq(&self, other: &Self) -> bool {self.title_.eq(&other.title_)}}

	fn add__(path1:&str, path0:String, o:&Opt_, src:&str, env:&code_::Env_) -> Result2_ {
		let path0 = if path0.is_empty() {
			path1.to_string()
		} else {path0};
		match fs::read_dir(path1) {
			Ok(paths) => {
				let mut paths2 = vec![];
				for path in paths {
					if let Ok(path) = path {
						let path2 = path.path();
						let mut path3 = path2.display().to_string();
						#[cfg(feature = "re")]
						if let Some(re) = &o.match_ {
							if !re.is_match(if o.P_ {&path3} else {
								path2.file_name().unwrap().to_str().unwrap()
							}) {
								continue;
							}
						}
						let mut path4 = path3[path0.len()..].to_string();
						loop {
							if path4.starts_with('/') {
								path4 = path4[1..].to_string();
							} else {break;}
						}
						let is_h = path4.starts_with('.');
						let mut typ = String::new();
						if path2.is_dir() {
							if o.s_ && !(o.n_ && path2.is_symlink()) {
								add__(&path3, path0.clone(), o, src, env)?;
								continue;
							}
							if o.a_d_ {
								continue;
							}
							if !o.f_ {
								path3.push('/');
							}
							typ.push('d');
						} else {
							if o.a_f_ && (!path2.is_symlink() || o.a_l_) {
								continue;
							}
						}
						if is_h && o.a_h_ {
							continue;
						}
						if path2.is_symlink() {
							typ.push('l');
						}
						paths2.push(Item_ {title_:path4, path_:path3, typ_:typ});
					}
				}
				paths2.sort();
				let mut only_b = false;
				for i in &paths2 {
					let q = Qv_::new2(Some(env.q.clone()));
					{
						let args = &mut as_mut_ref__!(q.args_);
						args.add__(i.title_.clone());
						as_ref__!(env.w).dunhao__(args);
						args.add__(i.path_.clone());
						as_ref__!(env.w).dunhao__(args);
						args.add__(i.typ_.clone());
					}
					let ret = eval_::hello__(&src, &mut code_::Env_::new2(t__(q), env));
					jump_::for_err2__(ret, &mut only_b)?;
					if only_b {break;}
				}
			}
			Err(_) => {}
		}
		ok__()
	}
	for idx in 0..args.len() {
		match idx {
			1 | 2 | 3 => {}
			_ => {
				add__(&args[idx], "".to_string(), &o, &args[1], env)?;
			}
		}
	}
	ok__()
}

#[no_mangle]
pub extern fn base64_file__(env:&code_::Env_) -> Result2_ {
	let q = as_ref__!(env.q);
	let args = {
		let args = as_ref__!(q.args_);
		args.to_vec__()
	};
	if_buzu__(args.len(), 1, 1)?;
	let ret = &mut as_mut_ref__!(env.ret);
	match File::open(&args[0]) {
		Ok(mut f) => {
			let mut buf = Vec::new();
			match f.read_to_end(&mut buf) {
				Ok(_) => {
					ret.add__(general_purpose::STANDARD_NO_PAD.encode(&buf));
				}
				Err(err) => {
					as_ref__!(env.w).dunhao__(ret);
					ret.add__(err);
				}
			}
		}
		Err(err) => {
			as_ref__!(env.w).dunhao__(ret);
			ret.add__(err);
		}
};
	ok__()
}

#[no_mangle]
pub extern fn exist_val__(env:&code_::Env_) -> Result2_ {
/*fn for_val__(env:&code_::Env_, cmp:impl Fn(&str, &str) -> bool, only1:bool) -> Result2_ {*/
	let q = as_ref__!(env.q);
	let args = &as_ref__!(q.args_);
	let a = &args.a_;
	let mut val = String::new();

	as_ref__!(a[0]).s__(&mut val);
	let cntn = !val.is_empty();
	val.clear();

	as_ref__!(a[2]).s__(&mut val);
	let only1 = !val.is_empty();
	val.clear();

	let mut idx = 4;
	let up_q = q.up_.clone();
	let mut q2 = up_q.clone();
	let mut first = true;
	while idx < a.len() {
		let i = as_ref__!(a[idx]);
		let mut b = false;
		let mut next = true;
		if i.dunhao__() {
			b = true;
		} else {
			match code_::qv4rem__(&i.rem_, |_| {false}, q2.unwrap(), env.w.clone()) {
				Ok(q3) => {
					q2 = q3;
					next = false;
				}
				Err(e) => return e,
			}
			i.s__(&mut val);
		}
		idx += 1;
		let mut b2 = idx >= a.len();
		if b || b2 {
			if first {first = false;} else {
				as_ref__!(env.w).dunhao__(&mut as_mut_ref__!(env.ret));
			}

			let mut has = false;
			qv_::for3__(q2.unwrap(), env.w.clone(), |q, _, _| -> Option<()> {
				let q = as_ref__!(q);
				for i in &q.vars_.a_ {
					let i = as_ref__!(i);
					let name = i.name__();
					let mut ret2 = result_::List_::new();
					if q.vars_.get__(name, false, false, &mut ret2) {
						if !ret2.is_empty() {
							let s = ret2.s__();
							if /*cmp(&s, &val)*/ if cntn {s.contains(&val)} else {s == val} {
								let mut ret = as_mut_ref__!(env.ret);
								let w = as_ref__!(env.w);
								if has {
									ret.add__(w.kws_.dunhao_.s_.clone());
								}
								if only1 {
									ret.add__(name);
									b2 = true;
									return Some(());
								} else {
									ret.add__(w.text__(name));
								}
								has = true;
							}
						}
					}
				}
				None
			}, next, false, false);

			if b2 {break;}

			q2 = up_q.clone();
			val.clear();
		}
	}
	ok__()
}
/*#[no_mangle]
pub extern fn exist_val__(env:&code_::Env_) -> Result2_ {
	for_val__(env, |s, val| s == val, false)
}
#[no_mangle]
pub extern fn first_exist_val__(env:&code_::Env_) -> Result2_ {
	for_val__(env, |s, val| s == val, true)
}
#[no_mangle]
pub extern fn cntn_val__(env:&code_::Env_) -> Result2_ {
	for_val__(env, |s, val| s.contains(val), false)
}
#[no_mangle]
pub extern fn first_cntn_val__(env:&code_::Env_) -> Result2_ {
	for_val__(env, |s, val| s.contains(val), true)
}*/

#[cfg(debug_assertions)]
#[no_mangle]
pub extern fn test__(env:&code_::Env_) -> Result2_ {
	let q = as_ref__!(env.q);
	let args = &as_ref__!(q.args_);
	let a = &args.a_;
	let mut idx = 6;
	let mut val = String::new();
	let mut ret = as_mut_ref__!(env.ret);
	let w = as_ref__!(env.w);
	as_ref__!(a[0]).s__(&mut val);
	as_ref__!(a[2]).s__(&mut val);
	as_ref__!(a[4]).s__(&mut val);
	while idx < a.len() {
		let i = as_ref__!(a[idx]);
		let b = i.dunhao__();
		if !b {
			i.s__(&mut val);
		}
		idx += 1;
		let b2 = idx >= a.len();
		if b || b2 {
			w.dunhao__(&mut ret);
			ret.add__(val.clone());
			if b2 {break;}
			val.clear();
		}
	}
	ok__()
}

#[cfg(test)]
mod tests {
	#[allow(unused_imports)]
    use super::*;

    #[test]
    fn it_works() {
    }
}
