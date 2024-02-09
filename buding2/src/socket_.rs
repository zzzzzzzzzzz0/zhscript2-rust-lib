use std::{os::unix::net::{UnixStream, UnixListener}, io::{BufReader, BufWriter, BufRead, Read, Write}, fs, process};
use zhscript2::{u_::*, as_ref__, as_mut_ref__};
use super::t_::*;

#[no_mangle]
pub extern fn sock_bind__(env:&code_::Env_) -> Result2_ {
	let q = env.q.clone();
	let q = as_ref__!(q);
	let args = {
		let args = as_ref__!(q.args_);
		args.to_vec__()
	};
	if_buzu__(args.len(), 2, 2)?;
	let addr = &args[0];
	let src = &args[1];

	let ret = &mut as_mut_ref__!(env.ret);
	match UnixListener::bind(addr) {
		Ok(ul) => {
			for i in ul.incoming() {
				match i {
					Ok(us) => {
						let mut br = BufReader::new(&us);
						let mut s = String::new();
						if br.read_line(&mut s).is_ok() {
							let q2 = Qv_::new2(q.up_.clone());
							{
								let args = &mut as_mut_ref__!(q2.args_);
								args.add__(s.trim());
							}
							let ret4 = t__(result_::List_::new());
							let ret3 = eval_::hello__(&src, &mut code_::Env_::new9(t__(q2), ret4.clone(), env));
							if ret3.is_err() {
								if let Err((i, i2, s, s2)) = &ret3 {
									if *i == jump_::QUIT_ && s.is_empty() {
										fs::remove_file(addr).unwrap();
										process::exit(*i2);
									}
									result2_::eprtn__(*i, *i2, s, s2);
								}
							}
							let ret2 = as_ref__!(ret4).to_vec__();
							if !ret2.is_empty() {
								let mut bw = BufWriter::new(us);
								bw.write_all(ret2[0].as_bytes()).unwrap();
							}
						}
					}
					Err(e) => ret.add__(e)
				}
			}
		}
		Err(e) => ret.add__(e)
	}
	ok__()
}

#[no_mangle]
pub extern fn sock_conn__(env:&code_::Env_) -> Result2_ {
	let q = env.q.clone();
	let q = as_ref__!(q);
	let args = as_ref__!(q.args_).to_vec__();
	if_buzu__(args.len(), 3, 3)?;
	let addr = &args[0];
	let s = &args[1];
	let src = &args[2];

	let ret = &mut as_mut_ref__!(env.ret);
	match UnixStream::connect(addr) {
		Ok(mut us) => {
			us.write_all(s.as_bytes()).unwrap();
			us.write_all(b"\n").unwrap();
			let mut s2 = String::new();
			us.read_to_string(&mut s2).unwrap();

			let q2 = Qv_::new2(q.up_.clone());
			{
				let args = &mut as_mut_ref__!(q2.args_);
				args.add__(s2);
			}
			let ret4 = t__(result_::List_::new());
			let ret3 = eval_::hello__(&src, &mut code_::Env_::new9(t__(q2), ret4.clone(), env));
			if ret3.is_err() {
				return ret3;
			}
			for i in &as_ref__!(ret4).a_ {
				ret.add4__(i.clone());
			}
		}
		Err(e) => ret.add__(e)
	}
	ok__()
}
