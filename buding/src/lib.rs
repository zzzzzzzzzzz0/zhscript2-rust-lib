//#![no_std]

use std::{fs::File, io::Read};
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
	let len = len__(&arg[0]) as i64;
	let mut start = stoi__(arg, 1)?;
	if start < 0 {
		start += len;
	}
	let mut end = stoi__(arg, 2)?;
	if end <= 0 {
		end += len;
	}
	if arg.len() > 3 {
		start += len__(&arg[3]) as i64
	}
	if start > len {
		return Ok(())
	}
	if start < 0 {
		start = 0
	}
	if arg.len() > 4 {
		end -= len__(&arg[4]) as i64
	}
	if end < 0 {
		return Ok(())
	}
	if end > len {
		end = len
	}
	let start = start as usize;
	let end = end as usize;
	let mut s = String::new();
	{
		let cs = arg[0].chars();
		/*for (idx, c) in cs.skip(start).enumerate() {
			if idx >= end - start {
				break
			}
			s.push(c)
		}*/
		let mut i = 0;
		for c in cs {
			i = i + if c.len_utf8() > 1 {2} else {1};
			if i <= start {continue}
			if i > end {break}
			s.push(c);
		}
	}
	if arg.len() > 5 {
		if start > 0 {
			s = arg[5].to_string() + &s;
		}
		if end < len as usize {
			s += &arg[5];
		}
	}
	ret.push(s);
	Ok(())
}
fn len__(s:&str) -> usize {
	//s.len()
	gjke4_sp_len__(s)
}

fn gjke4_sp_len__(s:&str) -> usize {
	let cs = s.chars();
	let mut cnt = 0;
	for c in cs {
		cnt = cnt + if c.len_utf8() > 1 {2} else {1};
	}
	cnt
}

#[cfg(test)]
mod tests {
	use super::*;
	#[test]
	fn gjke4_strmid() {
		let arg = ["1啊2哦3呃4咦5呜6吁", "3", "7", "", "", "…",];
		let arg:Vec<String> = arg.iter().map(|i| i.to_string()).collect();
		let mut ret = vec![];
		gjke4_strmid__(&arg, &mut ret).ok();
		assert_eq!(ret[0], "…2哦3…");
	}
	#[test]
	fn gjke4_strmid2() {
		let arg = ["1啊2哦3呃4咦5呜6吁", "-9"];
		let arg:Vec<String> = arg.iter().map(|i| i.to_string()).collect();
		let mut ret = vec![];
		gjke4_strmid__(&arg, &mut ret).ok();
		assert_eq!(ret[0], "4咦5呜6吁");
	}
	#[test]
	fn gjke4_sp_len() {
		let s = "1啊a";
		assert_eq!(5, s.len());
		assert_eq!(3, s.chars().count());
		assert_eq!(4, gjke4_sp_len__(s));
	}
}
