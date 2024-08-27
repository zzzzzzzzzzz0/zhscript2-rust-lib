use std::{fs::File, io::Read, io::Seek, io::SeekFrom};
use super::t_::*;


#[no_mangle]
pub extern fn stoc__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 1, usize::MAX)?;
	let mut ma = false;
	let mut fu = false;
	for idx in 1..arg.len() {
		for c in arg[idx].chars() {
			match c {
				'码' => ma = true,
				'附' => fu = true,
				_ => return Err(("无效：").to_string() + &c.to_string())
			}
		}
	}
	let cs = arg[0].chars();
	let mut fu2 = String::new();
	let mut fu3 = false;
	let mut fu4 = true;
	for c in cs {
		if fu {
			if c == '|' {
				fu3 = !fu3;
				continue;
			}
			if fu3 {
				fu2.push(c);
				continue;
			}
			if fu4 {
				fu4 = false;
			} else {
				ret.push(fu2.clone());
				if !fu2.is_empty() {
					fu2.clear();
				}
			}
		}
		ret.push(c.to_string());
		if ma {
			ret.push((c as u64).to_string());
		}
	}
	if fu {
		ret.push(String::new());
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

#[no_mangle]
extern fn cmp2__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 2, 2)?;
	let mut s1 = String::new();
	let mut s2 = String::new();
	let mut cs1 = arg[0].chars();
	let mut cs2 = arg[1].chars();
	loop {
		if let Some(c1) = cs1.next() {
			if let Some(c2) = cs2.next() {
				if c1 != c2 {
					s1.push(c1);
					s2.push(c2);
				}
			} else {
				s1.push(c1);
			}
		} else {
			while let Some(c2) = cs2.next() {
				s2.push(c2);
			}
			break;
		}
	}
	if s1 != s2 {
		ret.push(s1);
		ret.push(s2);
	}
	Ok(())
}

#[no_mangle]
extern fn mk_id__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 1, 2)?;
	let src = &arg[0];
	let typ = if arg.len() > 1 && !arg[1].is_empty() {arg[1].as_str()} else {"w"};
	match typ {
		"w" | "w2" => {
			let mut s = String::new();
			let mut cs = src.chars();
			let gang: bool = typ == "w2";
			let c2s = |c, first, s:&mut String| {
				let jinzhi = 26 + 26 + if first {0} else {10 + if gang {1} else {2}};
				if gang && !first && !s.ends_with('_') {
					s.push('_');
				}
				let mut cc = c as u32;
				loop {
					let mut i = cc % jinzhi;
					//-45 _95
					if cc < 256 {
						if !gang && cc >= 95 {i -= 1;}
						if i >= 45 {i -= 1;}
					}
					match i {
						62 => s.push('-'),
						63 => s.push('_'),
						_ =>
						if i < 26 {
							s.push(char::from_u32('a' as u32 + i).unwrap());
						} else if i >= 26 && i < 52 {
							s.push(char::from_u32('A' as u32 + i - 26).unwrap());
						} else {
							s.push(char::from_u32('0' as u32 + i - 52).unwrap());
						}
					}
					let i = cc / jinzhi;
					if i == 0 {break;}
					cc = i;
				}
				if gang {
					s.push('_');
				}
			};
			let mut first = true;
			while let Some(c) = cs.next() {
				if first {
					first = false;
					if !(c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
						c2s(c, true, &mut s);
						continue;
					}
				} else if c == '_' {
					if gang {
						c2s(c, false, &mut s);
						continue;
					}
				} else if !(c == '-'
						|| c >= '0' && c <= '9'
						|| c >= 'a' && c <= 'z'
						|| c >= 'A' && c <= 'Z') {
					c2s(c, false, &mut s);
					continue;
				}
				s.push(c);
			}
			ret.push(s);
		}
		"i" => {
			use std::hash::Hasher;
			let mut h = std::collections::hash_map::DefaultHasher::new();
			h.write(src.as_bytes());
			ret.push(h.finish().to_string());
		}
		_ => return Err(typ.to_string())
	}
	Ok(())
}
