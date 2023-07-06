use std::{fs::File, io::Read, io::Seek, io::SeekFrom};
use super::t_::*;


#[no_mangle]
pub extern fn stoc__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 1, usize::MAX)?;
	let mut ma = false;
	for idx in 1..arg.len() {
		let s = &arg[idx];
		match s.as_str() {
			"码" => ma = true,
			_ => {
				return Err(("无效：").to_string() + s)
			}
		}
	}
	let cs = arg[0].chars();
	for c in cs {
		ret.push(c.to_string());
		if ma {
			ret.push((c as u64).to_string());
		}
	}
	Ok(())
}

#[no_mangle]
extern fn is_jpeg__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 1, 1)?;
	match File::open(&arg[0]) {
		Ok(mut f) => {
			let mut buf = [0; 2];
			if let Err(e) = f.read(&mut buf) {
				return Err(e.to_string())
			}
			if buf[0] == 0xff && buf[1] == 0xd8 {} else {
				ret.push("0".to_string());
				return Ok(())
			}
			if let Err(e) = f.seek(SeekFrom::End(-2)) {
				return Err(e.to_string())
			}
			if let Err(e) = f.read(&mut buf) {
				return Err(e.to_string())
			}
			if buf[0] == 0xff && buf[1] == 0xd9 {} else {
				return Ok(())
			}
			ret.push("1".to_string());
			Ok(())
		}
		Err(e) => Err(e.to_string())
	}
}
