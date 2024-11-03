//#![no_std]

use std::{fs::File, io::Read, os::unix::fs::{PermissionsExt, MetadataExt}};
mod t_;
use t_::*;
mod num2fmtstr_;
mod other_;

#[no_mangle]
extern fn doscmd4_type__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	let mut buf = String::new();
	for i in arg {
		match File::open(i) {
			Ok(mut f) => {
				if let Err(e) = f.read_to_string(&mut buf) {
					return Err(e.to_string())
				}
			}
			Err(e) => return Err(e.to_string())
		}
	}
	ret.push(buf);
	Ok(())
}

#[no_mangle]
extern fn gjke4_replace__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	let mut buf = String::from(&arg[0]);
	let mut idx = 1;
	while idx < arg.len() {
		let from = &arg[idx];
		idx += 1;
		let to = if idx < arg.len() {&arg[idx]} else {""};
		buf = buf.replace(from, to);
		idx += 1;
	}
	ret.push(buf);
	Ok(())
}

#[no_mangle]
extern fn gjke4_strmid__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 1, 6)?;
	let len = arg[0].len() as i64;
	let mut start = stoi__(arg, 1)?;
	let start1 = if start < 0 {
		start += len;
		true
	} else {
		false
	};
	if arg.len() > 3 {
		start += arg[3].len() as i64
	}
	if start > len {
		return Ok(())
	}
	let mut end = stoi__(arg, 2)?;
	if end <= 0 {
		end += len;
	}
	if arg.len() > 4 {
		end -= arg[4].len() as i64
	}
	if end < 0 {
		return Ok(())
	}
	let start = if start < 0 {0} else {start as usize};
	let end = if end > len {len as usize} else {end as usize};
	let mut s = String::new();
	{
		let cs = arg[0].chars();
		let mut i = 0;
		for c in cs {
			i += c.len_utf8();
			if i <= start {
				continue;
			}
			if i > end {
				break;
			}
			s.push(c);
		}
	}
	if arg.len() > 5 {
		let ddd = &arg[5];
		if start1 && start > 0 {
			s = ddd.to_string() + &s;
		}
		if end < len as usize {
			s += ddd;
		}
	}
	ret.push(s);
	Ok(())
}

#[no_mangle]
extern fn gjke4_sp_len__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 1, 1)?;
	let cs = arg[0].chars();
	let mut cnt = 0;
	for c in cs {
		cnt = cnt + if c.len_utf8() > 1 {2} else {1};
	}
	ret.push(cnt.to_string());
	Ok(())
}

#[cfg(debug_assertions)]
#[no_mangle]
extern fn file4_fileinfo__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 2, 2)?;
	let filename = &arg[0];
	match File::open(filename) {
		Ok(f) => match f.metadata() {
			Ok(md) => {
				let fmt = &arg[1];
				let mut s = String::new();
				let cs = fmt.as_bytes();
				let mut i = 0;
				let e__ = |c| Err((c as char).to_string() + " 无效格式符");
				loop {
					if i >= cs.len() {
						break;
					}
					let c = cs[i];
					match c {
						b'%' => {
							i += 1;
							if i >= cs.len() {
								return e__(c);
							}
							let c = cs[i];
							match c {
								b's' => s.push_str(&md.len().to_string()),
								b'm' => s.push_str(&format!("{:x}", md.permissions().mode())),
								b'o' => s.push_str(&format!("{:o}", md.permissions().mode())),
								//b'M' => s.push_str(&format!("{:?}", md.permissions())),
								b't' => {
									i += 1;
									if i >= cs.len() {
										return e__(c);
									}
									let c = cs[i];
									match c {
										/*b'a' => if let Ok(tm) = md.accessed() {
											s.push_str(&format!("{:?}", tm));
										} else {return e__(c);}*/
										_ => return e__(c)
									}
								}
								b'i' => s.push_str(&md.ino().to_string()),
								b'B' => s.push_str(&md.blksize().to_string()),
								b'b' => s.push_str(&md.blocks().to_string()),
								b'r' => s.push_str(&md.rdev().to_string()),
								b'u' => s.push_str(&md.uid().to_string()),
								b'g' => s.push_str(&md.gid().to_string()),
								b'n' => s.push_str(&md.nlink().to_string()),
								b'd' => s.push_str(&md.dev().to_string()),
								b'%' => s.push(c as char),
								_ => return e__(c)
							}
						}
						_ => s.push(c as char),
					}
					i += 1;
				}
				ret.push(s);
			}
			Err(e) => return Err(e.to_string())
		}
		Err(e) => return Err(e.to_string())
	}
	Ok(())
}

#[cfg(test)]
mod tests {
	use super::*;
	#[test]
	fn gjke4_strmid() {
		let arg = ["1啊2哦3呃4咦5呜6吁", "4", "9", "", "", "…",];
		let arg:Vec<String> = arg.iter().map(|i| i.to_string()).collect();
		let mut ret = vec![];
		gjke4_strmid__(&arg, &mut ret).ok();
		assert_eq!(ret[0], "2哦3…");
	}
	#[test]
	fn gjke4_strmid2() {
		let arg = ["1啊2哦3呃4咦5呜6吁", "-12"];
		let arg:Vec<String> = arg.iter().map(|i| i.to_string()).collect();
		let mut ret = vec![];
		gjke4_strmid__(&arg, &mut ret).ok();
		assert_eq!(ret[0], "4咦5呜6吁");
	}
}
